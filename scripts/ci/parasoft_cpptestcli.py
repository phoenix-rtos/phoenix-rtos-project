#!/usr/bin/env python3

import argparse
import dataclasses
import platform
import subprocess
import json
import os

from typing import Any, Dict, Sequence, Set, Optional

@dataclasses.dataclass(eq=True, frozen=True, repr=True)
class CpptestPosition:
    line: int
    column: int

    def rdf(self) -> Dict[str, Any]:
        return {
            "line": self.line,
            "column": self.column,
        }


@dataclasses.dataclass(eq=True, frozen=True, repr=True)
class CpptestDiagnostic:
    path: str
    rule: str
    message: str
    start: CpptestPosition
    end: CpptestPosition

    def __str__(self) -> str:
        return f"{self.rule}: {self.message}\n{self.path}: {self.line}\n"

    def rdf(self) -> Dict[str, Any]:
        return {
            "message": self.message,
            "location": {
                "path": self.path,
                "range": {
                    "start": self.start.rdf(),
                    "end": self.end.rdf(),
                },
             },
            "code": {
                "value": self.rule,
            },
        }


class CpptestcliError(Exception):
    pass


def run_cpptestcli(target: str, compile_commands: str, files: Sequence[str]):
    cmd = [
        "cpptestcli",
        "-config", "user://MISRA C 2012",
        "-input", compile_commands,
        "-module", ".",
    ]

    if target == "armv7a7-imx6ull-evk":
        cmd.extend(["-compiler", "gcc_9-64_ARM"])
    elif target.startswith("arm"):
        cmd.extend(["-compiler", "gcc_9_ARM"])
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

    if proc.returncode != 0:
        print("Return code", proc.returncode)
        print("Stdout\n", proc.stdout)
    #print("Stderr\n", proc.stderr)

    with open("result.txt", "w") as f:
        f.write(proc.stdout)

    return proc.returncode, proc.stdout

#def run_cpptestcli(compile_commands: str):
#    with open("result.txt") as f:
#        return 0, f.read(), ""


#def iter_cpptest_result(output):
#    # yield here diagnostic
#
#    #error_re = r"(?P<rule>[\w-]+): (?P<message>.+)\n(?P<path>[\w\/\.-]+): (?P<line>\d+)"
#    # Try to find here and remove from text
#    error_re = r"(?P<rule>[\w-]+): (?P<message>.+)\n(?P<path>.+): (?P<line>\d+)\n"
#
#    for m in re.finditer(error_re, output):
#        d = m.groupdict()
#        yield CpptestDiagnostic(
#            path=d["path"],
#            line=int(d["line"]),
#            rule=d["rule"],
#            message=d["message"],
#        )


def iter_sarif_result(data: Dict):
    results = data["runs"][0]["results"]

    def remove_platform_prefix(s):
        prefix = f"file://{platform.node()}"
        return s[len(prefix):]

    for result in results:
        # TODO add proper parsing, add level keyword
        location = result["locations"][0]["physicalLocation"]
        path = remove_platform_prefix(location["artifactLocation"]["uri"])
        start = CpptestPosition(
            line=int(location["region"]["startLine"]),
            column=int(location["region"]["startColumn"]),
        )
        end = CpptestPosition(
            line=int(location["region"].get("endLine", start.line)),
            column=int(location["region"].get("endColumn", start.column))
        )

        yield CpptestDiagnostic(
            path=path,
            start=start,
            end=end,
            message=result["message"]["text"],
            rule=result["ruleId"],
        )


def run_cpptestcli_process(target:str, compile_commands: str, files: Sequence[str]) -> Set:
    try:
        rc, output = run_cpptestcli(target, compile_commands, files)
    except CpptestcliError:
        print("file cpptescli not found")
        return set()

    if rc != 0:
        print("error, rc not zero")
        return set()

    with open("reports/report.sarif", "r") as f:
        results = json.load(f)

    l = [res for res  in iter_sarif_result(results)]
    diagnostics = set(l)


    #l = [res for res in iter_cpptest_result(stdout)]
    #cnt = collections.Counter(l)
    #with open("next.txt", "w") as f:
    #    for key, c in cnt.items():
    #        if c != 1:
    #            f.write(f"Count {c}:\n{key}")


    #results = set(l)
    print(f"len or diagnostics: {len(diagnostics)} / {len(l)}")

    #error_re = r"(?P<rule>[\w-]+): (?P<message>.+)\n(?P<path>.+): (?P<line>\d+)\n"
    #res = re.sub(error_re, "", stdout)
    #res = '\n'.join([line for line in res.split("\n") if line.strip()])
    #with open("another.txt", "w+") as f:
    #    f.write(res)

    return diagnostics


def convert_arguments_to_command(record):
    # Propably we do not need this if here, check it later
    if "arguments" in record:
       record["command"] = " ".join(record["arguments"])
       del record["arguments"]


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

    with open(compile_db, 'r') as f:
        data = json.load(f)

    for record in data:
        convert_arguments_to_command(record)
        if submodule:
            fix_path_submodule(record, submodule, workdir)
        else:
            fix_path_project(record, workdir)

    with open(compile_db, 'w') as f:
        f.write(json.dumps(data, indent=4))


def parse_opts() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Blablabla for now")

    #TARGETS = "armv7a7-imx6ull-evk,armv7a9-zynq7000-qemu,armv7a9-zynq7000-zedboard,armv7a9-zynq7000-zturn,armv7m4-stm32l4x6-nucleo,armv7m7-imxrt106x-evk,armv7m7-imxrt117x-evk,host-generic-pc,ia32-generic-pc,ia32-generic-qemu,riscv64-generic-qemu,riscv64-generic-spike"
    TARGETS = "ia32-generic-qemu,armv7m7-imxrt106x-evk,armv7a7-imx6ull-evk"

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
    print(opts.files)
    #opts.files = opts.files.split()

    return opts

def main() -> None:
    opts = parse_opts()

    #print(f"Starting N={opts.j} multiprocesses for targets {opts.targets}")

    #absent = absent_cdb(opts.targets)
    #if len(absent) != 0:
    #    print("missing files:", " ".join(absent))
    #    sys.exit(1)

    diagnostics = set()

    for target in opts.targets:
        compile_db = f"_build/{target}/compile_commands.json"
        fix_compile_db(compile_db, opts.fix_compile_db)
        diagnostics |= run_cpptestcli_process(target, compile_db, opts.files)

    print(f"Total: {len(diagnostics)}")

main()
