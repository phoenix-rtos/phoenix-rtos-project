#!/usr/bin/env python3
"""Fixes paths in compile_commands.json to be relative and removes .buildroot GH hack."""

import argparse
import json
import os
import sys

# Example JSON file when building "libphoenix" module
EXAMPLE_JSON = """
[
    {
        "arguments": [
            "i386-pc-phoenix-gcc",
            "-c",
            "-D__TARGET_IA32",
            "-D__CPU_GENERIC",
            "-I/home/runner/work/libphoenix/libphoenix/.buildroot/phoenix-rtos-project/phoenix-rtos-lwip/include",
            "-I/home/runner/work/libphoenix/libphoenix/.buildroot/phoenix-rtos-project/_projects/ia32-generic-qemu",
            "-m32",
            "-march=i586",
            "-mtune=generic",
            "-mno-mmx",
            "-mno-sse",
            "-fno-pic",
            "-fno-pie",
            "-fomit-frame-pointer",
            "-fno-builtin-malloc",
            "--sysroot=/home/runner/work/libphoenix/libphoenix/.buildroot/phoenix-rtos-project/_build/ia32-generic-qemu/sysroot/",
            "-B/home/runner/work/libphoenix/libphoenix/.buildroot/phoenix-rtos-project/_build/ia32-generic-qemu/sysroot/lib/",
            "-iprefix",
            "/home/runner/work/libphoenix/libphoenix/.buildroot/phoenix-rtos-project/_build/ia32-generic-qemu/sysroot/",
            "-I/home/runner/work/libphoenix/libphoenix/.buildroot/phoenix-rtos-project/_build/ia32-generic-qemu/include/",
            "-fmacro-prefix-map=/home/runner/work/libphoenix/libphoenix/.buildroot/phoenix-rtos-project/=",
            "-ffunction-sections",
            "-fdata-sections",
            "-Wall",
            "-Wstrict-prototypes",
            "-Wundef",
            "-Werror",
            "-fno-common",
            "-fno-strict-aliasing",
            "-ggdb3",
            "-O2",
            "-Wundef",
            "-Iinclude",
            "-Ilib-lwip/src/include",
            "-I/home/runner/work/libphoenix/libphoenix/.buildroot/phoenix-rtos-project/_projects/ia32-generic-qemu/lwip",
            "-Wno-char-subscripts",
            "-Wno-format-zero-length",
            "-o",
            "/home/runner/work/libphoenix/libphoenix/.buildroot/phoenix-rtos-project/_build/ia32-generic-qemu/phoenix-rtos-lwip/lib-lwip/src/core/ipv4/etharp.o",
            "lib-lwip/src/core/ipv4/etharp.c"
        ],
        "directory": "/home/runner/work/libphoenix/libphoenix/.buildroot/phoenix-rtos-project/phoenix-rtos-lwip",
        "file": "lib-lwip/src/core/ipv4/etharp.c"
    },
    {
        "arguments": [
            "i386-pc-phoenix-gcc",
            "-c",
            "-D__TARGET_IA32",
            "-D__CPU_GENERIC",
            "-I/home/runner/work/libphoenix/libphoenix/.buildroot/phoenix-rtos-project/_projects/ia32-generic-qemu",
            "-m32",
            "-march=i586",
            "-mtune=generic",
            "-mno-mmx",
            "-mno-sse",
            "-fno-pic",
            "-fno-pie",
            "-fomit-frame-pointer",
            "-fno-builtin-malloc",
            "--sysroot=/home/runner/work/libphoenix/libphoenix/.buildroot/phoenix-rtos-project/_build/ia32-generic-qemu/sysroot/",
            "-B/home/runner/work/libphoenix/libphoenix/.buildroot/phoenix-rtos-project/_build/ia32-generic-qemu/sysroot/lib/",
            "-iprefix",
            "/home/runner/work/libphoenix/libphoenix/.buildroot/phoenix-rtos-project/_build/ia32-generic-qemu/sysroot/",
            "-I/home/runner/work/libphoenix/libphoenix/.buildroot/phoenix-rtos-project/_build/ia32-generic-qemu/include/",
            "-fmacro-prefix-map=/home/runner/work/libphoenix/=",
            "-ffunction-sections",
            "-fdata-sections",
            "-Wall",
            "-Wstrict-prototypes",
            "-Wundef",
            "-Werror",
            "-fno-common",
            "-fno-strict-aliasing",
            "-ggdb3",
            "-O2",
            "-Iinclude",
            "-fno-builtin-malloc",
            "-o",
            "/home/runner/work/libphoenix/libphoenix/.buildroot/phoenix-rtos-project/_build/ia32-generic-qemu/libphoenix/string/strdup.o",
            "string/strdup.c"
        ],
        "directory": "/home/runner/work/libphoenix/libphoenix",
        "file": "string/strdup.c"
    }
]
"""

def get_project_paths(data: list[dict], ROOT_REPO: str) -> tuple[str, str]:
    """Detects root path of the project and repo name from compile_commands.json data.

    Handles only submodule build case:
      - "DIR/[repo]/.buildroot/[ROOT_REPO]/**" <- project root
      - "DIR/[repo]/" <- repo

    See ci-submodule.yml for details on how the build is set up in GH actions.
    """
    root_path = ""
    repos = set()

    for record in data:
        if ROOT_REPO in record["directory"]:
            if not root_path:
                root_path = record["directory"].split(ROOT_REPO)[0] + ROOT_REPO
        else:
            repo = record["directory"].split(os.sep)[-1]
            repos.add(repo)

    assert len(repos) == 1, f"Expected exactly one repo, found: {repos}"
    return root_path, repos.pop()

def fix_compile_commands(data: list[dict], verbose: bool = False) -> None:
    """We're changing the paths as in normal superproject build:
     - use relative paths instead of absolute ones, skip ROOT_REPO part of the path
     - [repo]/.buildroot/[ROOT_REPO] -> [ROOT_REPO]/[repo]"""
    ROOT_REPO = "phoenix-rtos-project"
    CWD = os.getcwd()

    root_path, repo = get_project_paths(data, ROOT_REPO)

    if verbose:
        print(f"Detected root path: {root_path}", file=sys.stderr)
        print(f"Detected repo: {repo}", file=sys.stderr)


    for record in data:
        if ROOT_REPO in record["directory"]:
            record["directory"] = record["directory"].replace(root_path, CWD)
        else:
            record["directory"] = CWD + "/" + repo

        for i, arg in enumerate(record["arguments"]):
            if ROOT_REPO in arg: # path is a part of argument
                fixed_arg = arg.replace(root_path, CWD)
                record["arguments"][i] = fixed_arg


def parse_opts() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Fix compile commands paths to be relative")

    parser.add_argument("--verbose", action="store_true", default=False)
    parser.add_argument("file")

    opts = parser.parse_args()
    return opts


def main() -> None:
    opts = parse_opts()

    with open(opts.file, "r") as f:
        data = json.load(f)

    fix_compile_commands(data, opts.verbose)
    json.dump(data, fp=sys.stdout, indent=4)

def test() -> None:
    data = json.loads(EXAMPLE_JSON)
    fix_compile_commands(data, verbose=False)
    print(json.dumps(data, indent=4))

if __name__ == "__main__":
    # test()
    main()
