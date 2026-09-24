#!/usr/bin/env python3
#
# Script to write to Flash memory on STM32N6 boards over SWD connection.
#
# Copyright 2026 Phoenix Systems
# Author: Jacek Maksymowicz
#

import argparse
import os
from pathlib import Path
import sys
from typing import List, Optional, Tuple
import logging
import select
import socket
import subprocess
import time

"""
OpenOCD is used to load plo image into RAM then open an RTT connection to it. Writing to Flash is performed by plo.
Commands to plo are sent over RTT, file contents are transferred to RAM-disk then copied to Flash.
Use of RAM-disk rather than phoenixd over RTT helps achieve higher transfer speeds. The script also interleaves
erase operations with uploads into RAM to further improve speeds.

Board must be in development boot mode in order for this script to work.
"""

DEFAULT_SPEED = 24000


# Read socket until prompt is found and drain remaining data.
def socket_read_drain(s: socket.socket, prompt: bytes, timeout: int = 5):
    deadline = time.monotonic() + timeout
    buffer = b""
    new = b""
    while True:
        remaining = deadline - time.monotonic()
        if remaining <= 0:
            raise TimeoutError()

        ready, _, _ = select.select([s], [], [], remaining)
        if not ready:
            raise TimeoutError()

        chunk = s.recv(512)
        if not chunk:
            break

        new += chunk
        if prompt in new:
            buffer += new
            break
        else:
            idx = new.rfind(b"\n")
            if idx > 0:
                buffer += new[: idx + 1]
                new = new[idx + 1 :]

    # Drain remaining data from socket
    while True:
        ready, _, _ = select.select([s], [], [], 0)
        if not ready:
            break

        chunk = s.recv(512)
        if not chunk:
            break

        buffer += chunk

    return buffer


class InteractiveProcess:
    def __init__(self, cmd: List[Optional[str]]):
        self.cmd = list(filter(None, cmd))
        self.process = None

    def run(self):
        self.process = subprocess.Popen(
            self.cmd,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            text=True,
            bufsize=1,
        )

    def stop(self):
        if self.process is not None:
            self.process.terminate()

    def wait_for_prompt(self, prompt: str, timeout: int = 5):
        if self.process is None:
            raise RuntimeError(f"{self.cmd[0]} not running")

        assert self.process.stdout is not None
        deadline = time.monotonic() + timeout
        while True:
            remaining = deadline - time.monotonic()
            if remaining <= 0:
                self.process.terminate()
                raise TimeoutError(f"{self.cmd[0]} timed out")

            ready, _, _ = select.select([self.process.stdout], [], [], remaining)
            if not ready:
                self.process.terminate()
                raise TimeoutError(f"{self.cmd[0]} timed out")

            poll = self.process.poll()
            if poll is not None:
                raise RuntimeError(f"{self.cmd[0]} exited unexpectedly with code {poll}")

            line = self.process.stdout.readline()
            if not line:
                # EOF reached - program is likely about to exit. Give it a moment to do so.
                time.sleep(0.1)
                poll = self.process.poll()
                retcode_str = f" with code {poll}" if poll is not None else ""
                raise RuntimeError(f"{self.cmd[0]} exited unexpectedly{retcode_str}")
            elif prompt in line:
                break
            elif line.strip():
                logging.debug(line.strip())


class STM32N6Flasher:
    READY_TAG = ">>ready<<"
    PROMPT = b"(plo)%"
    DEFAULT_PLO_FILENAME = "plo-ram.elf"
    OPENOCD_PORT = 4444
    COMMAND_PORT = 18021
    # TODO: the two timeouts below could be calculated per-operation
    UPLOAD_TIMEOUT = 10
    PLO_CMD_TIMEOUT = 30

    def __init__(
        self,
        scripts_dir: Path,
        boot_dir: Path,
        plo_path: Optional[Path] = None,
        adapter_speed: Optional[int] = None,
    ) -> None:
        # Nice to be configurable
        self.ramdisk_name = "ramdisk"
        self.ramdisk_addr = 0x34100000
        self.ramdisk_size = 0x00080000

        self.boot_dir = boot_dir.absolute()
        if plo_path is None:
            plo_path = Path(self.DEFAULT_PLO_FILENAME)

        plo_path = plo_path if plo_path.is_absolute() else self.boot_dir / plo_path
        scripts_dir = scripts_dir.absolute()
        base_script = scripts_dir / "openocd/stm32n6/stm32n6x.cfg"
        rtt_script = scripts_dir / "stm32n6-plo-rtt.cfg"

        for x in (plo_path, base_script, rtt_script):
            if not x.is_file():
                raise FileNotFoundError(x)

        self.ocd = InteractiveProcess(
            [
                "openocd",
                "-f",
                "interface/stlink.cfg",
                "-f",
                str(base_script),
                "-c" if adapter_speed else None,
                f"adapter speed {adapter_speed}" if adapter_speed else None,
                "-c",
                f'set PLO_PATH "{plo_path.as_posix()}"',
                "-f",
                str(rtt_script),
                "-c",
                f"puts {self.READY_TAG}",
            ]
        )

    def __enter__(self):
        try:
            self.ocd.run()
            self.ocd.wait_for_prompt(self.READY_TAG)
            logging.debug("OpenOCD ready")

            self.ocd_socket = socket.create_connection(("localhost", self.OPENOCD_PORT), timeout=5)
            self.cmd_socket = socket.create_connection(("localhost", self.COMMAND_PORT), timeout=5)
        except:
            self.__exit__(*sys.exc_info())
            raise

        return self

    def __exit__(self, type, value, traceback):
        if hasattr(self, "cmd_socket"):
            self.cmd_socket.close()

        if hasattr(self, "ocd_socket"):
            self.ocd_socket.close()

        self.ocd.stop()

    def read_until_prompt(self):
        return socket_read_drain(self.cmd_socket, self.PROMPT, self.PLO_CMD_TIMEOUT)

    def upload_chunk(self, file: Path, start: int, end: int):
        logging.info("Uploading %s [0x%x : 0x%x]", file.name, start, end)
        assert end > start
        # The `address` argument of `load_image` works unintuitively - it moves the start of the whole file,
        # not just the chunk we want to copy. Bounds check to ensure it's never negative.
        if self.ramdisk_addr < start:
            raise RuntimeError("Starting offset in file is too large")

        cmd = f'load_image "{file.absolute().as_posix()}" 0x{self.ramdisk_addr - start:08x} bin 0x{self.ramdisk_addr:08x} {end - start}\n'
        cmd += f"puts {self.READY_TAG}\n"
        self.ocd_socket.send(cmd.encode())
        socket_read_drain(self.ocd_socket, self.READY_TAG.encode(), self.UPLOAD_TIMEOUT)

    def issue_copy_command(self, flash_name: str, part_offset: int, chunk_size: int):
        cmd_copy = f"copy {self.ramdisk_name} 0 0x{chunk_size:x} {flash_name} 0x{part_offset:x} 0x{chunk_size:x}"
        logging.debug('issuing "%s"', cmd_copy)
        self.cmd_socket.send(cmd_copy.encode("ascii") + b"\n")
        data = self.read_until_prompt()
        data_lines = data.decode().lower().splitlines()
        if any("error" in line for line in data_lines[1:]):
            raise RuntimeError(data.decode())

    def issue_erase_command(self, flash_name: str, flash_block_size: int, part_offset: int, chunk_size: int):
        bs = flash_block_size
        chunk_size_round = ((chunk_size + bs - 1) // bs) * bs
        cmd_erase = f"erase {flash_name} 0x{part_offset:x} 0x{chunk_size_round:x}"
        logging.debug('issuing "%s"', cmd_erase)
        self.cmd_socket.send(cmd_erase.encode("ascii") + b"\n")

    def wait_for_erase(self):
        logging.debug("waiting for erase end")
        data = self.read_until_prompt()
        data_str = data.decode().lower()
        if "erased " not in data_str or "error" in data_str:
            raise RuntimeError(data.decode())

    def write_partition(self, flash_name: str, flash_block_size: int, part_path: Path, offset: int):
        written = 0
        file_size = part_path.stat().st_size
        while written < file_size:
            to_write = min(file_size - written, self.ramdisk_size)
            self.read_until_prompt()
            # Interleave erase and upload for better performance
            self.issue_erase_command(flash_name, flash_block_size, offset + written, to_write)
            self.upload_chunk(part_path, written, written + to_write)
            self.wait_for_erase()
            self.issue_copy_command(flash_name, offset + written, to_write)
            written += to_write
            self.cmd_socket.send(b"\n")  # Do this to print out another prompt


def parse_args():
    fc = argparse.ArgumentDefaultsHelpFormatter
    scripts_dir = Path(os.path.realpath(__file__)).parent
    boot_dir = os.environ.get("PREFIX_BOOT")
    description = "Write to Flash memory on STM32N6 boards over SWD connection using upload to RAM-disk."
    parser = argparse.ArgumentParser("STM32N6 Flasher via RAM-disk", formatter_class=fc, description=description)
    parser.add_argument("--verbose", help="Verbose logging", action="store_true")
    parser.add_argument(
        "--boot-dir", help="Path to _boot directory", type=Path, default=boot_dir, required=(boot_dir is None)
    )
    parser.add_argument(
        "--plo-path", help="Path to `plo-ram.elf` file. Must be absolute or relative to _boot directory.", type=Path
    )
    parser.add_argument("--scripts-dir", help="Path to scripts directory", type=Path, default=scripts_dir)
    parser.add_argument("--adapter-speed", help="Debug adapter clock speed in kHz", type=int, default=DEFAULT_SPEED)

    subparsers = parser.add_subparsers(dest="action")

    # In the future we want to expand this to automatically retrieve info either from nvm.yaml
    # or expand the build process to produce a "manifest file" containing flash params and list of partitions.
    # For now require user to provide parameters manually.
    file_parser = subparsers.add_parser("files", help="Write file to selected Flash partition")
    file_parser.add_argument("--flash-name", help="Name of the target Flash device", type=str, default="flash0")
    file_parser.add_argument(
        "--block-size", help="Block size of the target Flash device", type=lambda x: int(x, 0), default=4096
    )

    def pair_parser(arg: str):
        path_str, offs_str = arg.rsplit(":", maxsplit=1)
        return Path(path_str), int(offs_str, 0)

    path_offset_help = """
    Pairs of `path:offset`.  Path to file to be written and byte offset in Flash where it should be written.
    Path must be absolute or relative to _boot directory.
    """
    file_parser.add_argument(
        "path_offset",
        help=path_offset_help,
        type=pair_parser,
        nargs="+",
    )

    args = parser.parse_args()
    return args


def flasher_file_action(args):
    parts: List[Tuple[Path, int]] = []
    for path, offset in args.path_offset:
        path: Path = path
        if not path.is_absolute():
            path = args.boot_dir / path

        if not path.is_file():
            raise FileNotFoundError(path)

        parts.append((path, offset))

    total_size = 0
    with STM32N6Flasher(args.scripts_dir, args.boot_dir, args.plo_path, args.adapter_speed) as flasher:
        logging.info("Flashing start")
        start = time.time()
        for path, offset in parts:
            total_size += path.stat().st_size
            flasher.write_partition(args.flash_name, args.block_size, path, offset)

        elapsed = time.time() - start

    logging.info("Wrote %u bytes in %.1f seconds (%.2f KiB/s)", total_size, elapsed, ((total_size / 1024) / elapsed))


if __name__ == "__main__":
    args = parse_args()
    logging.basicConfig(format="%(levelname)s: %(message)s", level=logging.INFO)
    if args.verbose:
        logging.getLogger().setLevel(logging.DEBUG)

    if args.action is None:
        raise RuntimeError("No action selected")
    elif args.action == "files":
        flasher_file_action(args)
    else:
        raise RuntimeError(f"Action {args.action} not recognized")
