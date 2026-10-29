#!/usr/bin/env python3
"""pico-3d-mouse build orchestrator: off-target host TDD suite and RP2040 firmware."""

import argparse
import os
import shlex
import subprocess
import sys


def run(cmd: str, cwd: str) -> None:
    print(f"+ {cmd}", flush=True)
    subprocess.run(cmd, cwd=cwd, shell=True, check=True)


def ensure_nix_shell(repo_root: str) -> None:
    """Re-exec inside `nix develop` once if not already in the devShell."""
    if os.environ.get("PICO_SDK_PATH"):
        return
    quoted_args = " ".join(shlex.quote(a) for a in sys.argv)
    cmd = ["nix", "develop", repo_root, "--command", "python3"] + sys.argv
    os.execvp("nix", cmd)


def build_host(repo_root: str, run_tests: bool, enable_asan: bool) -> None:
    asan_flag = "ON" if enable_asan else "OFF"
    cfg_cmd = f"cmake -S . -B build -G Ninja -DP3D_BUILD_TESTS=ON -DP3D_ENABLE_ASAN={asan_flag}"
    run(cfg_cmd, repo_root)
    run("cmake --build build", repo_root)
    if run_tests:
        run("ctest --test-dir build --output-on-failure", repo_root)


def build_firmware(repo_root: str) -> None:
    cfg_cmd = "cmake -S firmware -B firmware/build -G Ninja"
    run(cfg_cmd, repo_root)
    run("cmake --build firmware/build", repo_root)


def main() -> int:
    parser = argparse.ArgumentParser(
        description="Build off-target host tests and RP2040 target firmware."
    )
    parser.add_argument(
        "-t", "--test",
        action="store_true",
        help="Run off-target host unit tests after building host targets.",
    )
    parser.add_argument(
        "--host-only",
        action="store_true",
        help="Build (and optionally test) the off-target host suite only.",
    )
    parser.add_argument(
        "--target-only",
        action="store_true",
        help="Build the RP2040 target UF2 firmware only.",
    )
    parser.add_argument(
        "--asan",
        action="store_true",
        help="Enable AddressSanitizer and UndefinedBehaviorSanitizer on host build.",
    )
    parser.add_argument(
        "--clean",
        action="store_true",
        help="Remove build/ and firmware/build/ before building.",
    )
    args = parser.parse_args()
    repo_root = os.path.dirname(os.path.abspath(__file__))

    if args.host_only and args.target_only:
        print("error: cannot combine --host-only and --target-only", file=sys.stderr)
        return 2

    ensure_nix_shell(repo_root)

    if args.clean:
        run("rm -rf build firmware/build", repo_root)

    default_all = not args.host_only and not args.target_only

    if not args.target_only:
        build_host(repo_root, run_tests=(args.test or default_all), enable_asan=args.asan)

    if not args.host_only:
        build_firmware(repo_root)

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
