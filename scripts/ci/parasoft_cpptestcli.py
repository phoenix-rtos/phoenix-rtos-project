#!/usr/bin/env python3

import argparse
import dataclasses
import platform
import subprocess
import json
import os

from typing import Any, Dict, Generator, Sequence, Set, Optional

CPPTESTCLI_VERSION = "2022.1.0"


@dataclasses.dataclass(eq=True, frozen=True, repr=True)
class CpptestPosition:
    line: Optional[int]
    column: Optional[int]

    def rdf(self) -> Dict[str, Any]:
        data = {}

        if self.line is not None:
            data["line"] = self.line

        if self.column is not None:
            data["column"] = self.column

        return data


@dataclasses.dataclass(eq=True, frozen=True, repr=True)
class CpptestDiagnostic:
    path: str
    rule: str
    message: str
    start: CpptestPosition
    end: Optional[CpptestPosition]

    def __str__(self) -> str:
        return f"{self.rule}: {self.message}\n{self.path}: {self.start.line}\n"

    def rdf(self) -> Dict[str, Any]:
        data = {
            "message": self.message,
            "location": {
                "path": self.path,
                "range": {
                    "start": self.start.rdf(),
                },
            },
            "code": {
                "value": self.rule,
            },
        }

        if self.end is not None:
            data["range"]["end"] = self.end.rdf()

        return data


class CpptestcliError(Exception):
    pass


def run_cpptestcli(target: str, compile_commands: str, files: Sequence[str]):
    cmd = [
        "cpptestcli",
        "-config",
        "builtin://MISRA C 2023 (MISRA C 2012)",
        "-input",
        compile_commands,
        "-module",
        ".",
        "-property",
        "report.format=sarif"
    ]
    # TODO - add other targets compilers
    if target.startswith("armv7"):
        cmd.extend(["-compiler", "gcc_14-aarch32"])
    else:
        cmd.extend(["-compiler", "gcc_9"])

    for file in files:
        cmd.extend(["-resource", file])

    print(f"Run {cmd}")

    try:
        proc = subprocess.run(
            cmd,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            encoding="utf-8",
        )
    except FileNotFoundError as e:
        raise CpptestcliError("Failed to run cpptestcli - make sure it's installed") from e

    return proc.returncode, proc.stdout


def iter_sarif_result(data: Dict):
    def remove_platform_prefix(s):
        prefix = f"file://{platform.node()}"
        return s[len(prefix) :]

    # Cpptestcli should generate only one run as a result
    try:
        run = data["runs"][0]
        results = data["runs"][0]["results"]
    except (IndexError, KeyError) as e:
        raise CpptestcliError("sarif report does not contain results") from e

    tool = run["tool"]["driver"]

    if tool["name"] != "C/C++test":
        raise CpptestcliError("sarif report looks like it wasn't done by cpptestcli")

    if tool["semanticVersion"] != CPPTESTCLI_VERSION:
        raise CpptestcliError(f"cpptestcli is version {tool['semanticVersion']}, not {CPPTESTCLI_VERSION}")

    # We do not parse run.tool.driver.rules property, as the cpptest report do not use rulesIdx
    for result in results:
        try:
            # It should be only one physicalLocation
            location = result["locations"][0]
            # Cpptest use single uri for location
            path = remove_platform_prefix(location["physicalLocation"]["artifactLocation"]["uri"])
            # TODO: is there always startCol?
            # If there is no region then the scope is the entire artifact
            region = location["region"]
            start = CpptestPosition(
                line=int(region["startLine"]),
                column=int(region["startColumn"]) if "startColumn" in region else None,
            )
            # Make end optional? Now endCol should not be equal to startCol
            end = None
            if "endLine" in region or "endColumn" in region:
                end = CpptestPosition(
                    line=int(region["endLine"]) if "endLine" in region else None,
                    column=int(region["endColumn"]) if "endColumn" in region else None,
                )

            yield CpptestDiagnostic(
                path=path,
                start=start,
                end=end,
                message=result["message"]["text"],
                rule=result["ruleId"],
            )
            # For now ignore CodeFlow object
        except (IndexError, KeyError) as e:
            raise CpptestcliError from e


def run_cpptestcli_process(
    target: str, compile_commands: str, files: Sequence[str]
) -> Generator[CpptestDiagnostic, None, None]:

    rc, output = run_cpptestcli(target, compile_commands, files)
    if rc != 0:
        raise CpptestcliError(f"error, return code: {rc}, output: {output}")

    with open("reports/report.sarif", "r") as f:
        results = json.load(f)

    return iter_sarif_result(results)


def fix_path_submodule(record, submodule, workdir):
    # For now base dir is hardcode but we can detect it reading db (but at least two times)
    # We have to find two paths, first
    # First submodule have path /github/workspace
    # And a project has /github/workspace/.buildroot/phoenix-rtos-project

    basedir = "/github/workspace/.buildroot/phoenix-rtos-project"
    record["command"] = record["command"].replace(basedir, workdir)

    if record["directory"] == "/github/workspace":
        # That means this record is from submodule
        basedir = record["directory"]
        submoduledir = workdir + "/" + submodule
        record["directory"] = record["directory"].replace(basedir, submoduledir)
        record["command"] = record["command"].replace(basedir, submoduledir)
        record["command"] = record["command"].replace("workspace", submodule)
    else:
        record["directory"] = record["directory"].replace(basedir, workdir)


def fix_path_project(record, workdir):
    # subdir is usually submodule but it can be also _user dir
    basedir, _ = os.path.split(record["directory"])
    record["command"] = record["command"].replace(basedir, workdir)
    record["directory"] = record["directory"].replace(basedir, workdir)


def fix_compile_db(compile_db: str, submodule: Optional[str] = None):
    data = {}
    workdir = os.getcwd()

    with open(compile_db, "r") as f:
        data = json.load(f)

    for record in data:
        if submodule:
            fix_path_submodule(record, submodule, workdir)
        else:
            fix_path_project(record, workdir)

    with open(compile_db, "w") as f:
        f.write(json.dumps(data, indent=4))


def parse_opts() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Blablabla for now")

    # TARGETS = "armv7a7-imx6ull-evk,armv7a9-zynq7000-qemu,armv7a9-zynq7000-zedboard,armv7a9-zynq7000-zturn,armv7m4-stm32l4x6-nucleo,armv7m7-imxrt106x-evk,armv7m7-imxrt117x-evk,host-generic-pc,ia32-generic-pc,ia32-generic-qemu,riscv64-generic-qemu,riscv64-generic-spike"
    #TARGETS = "ia32-generic-qemu,armv7m7-imxrt106x-evk,armv7a7-imx6ull-evk"
    TARGETS = "armv7a9-zynq7000-qemu"

    parser.add_argument("--targets", type=str, default=TARGETS)
    parser.add_argument("--quiet", action="store_true", default=False)
    parser.add_argument("--fix-compile-db", type=str)
    parser.add_argument("-j", type=int, default=2)
    parser.add_argument("files", nargs="*", default=None)

    opts = parser.parse_args()
    opts.targets = opts.targets.split(",")
    opts.j = max(opts.j, len(opts.targets))

    if opts.fix_compile_db == "phoenix-rtos-project":
        opts.fix_compile_db = None

    # TODO
    opts.files = [file for s in opts.files for file in s.split()]
    print("files", opts.files)
    if len(opts.files) == 0:
        print("exit...")
        exit(1)
        # For now exit here

    # opts.files = opts.files.split()

    return opts


def compile_db_path(target: str) -> str:
    return f"_build/{target}/compile_commands.json"


def main() -> None:
    opts = parse_opts()

    # print(f"Starting N={opts.j} multiprocesses for targets {opts.targets}")

    # absent = absent_cdb(opts.targets)
    # if len(absent) != 0:
    #    print("missing files:", " ".join(absent))
    #    sys.exit(1)

    diagnostics = set()

    for target in opts.targets:
        #fix_compile_db(compile_db_path(target), opts.fix_compile_db)
        result_generator = run_cpptestcli_process(target, compile_db_path(target), opts.files)
#        diagnostics.update(result_generator)
#
#    print(f"Total: {len(diagnostics)}")
#    # Convert to rdf format
#    rdf = diagnostics_to_rdfjson(diagnostics)
#
#    with open("rdf.json", "w") as f:
#        f.write(rdf)


if __name__ == "__main__":
    main()
