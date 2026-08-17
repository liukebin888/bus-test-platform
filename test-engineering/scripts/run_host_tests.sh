#!/usr/bin/env bash
# run_host_tests.sh - Run all hardware-independent host tests (SW + FW).
#
# Requires: cmake >= 3.24, ninja or make, any C++17 / C11 compiler.
# In the WorkBuddy sandbox without a system toolchain, use the zig
# bootstrap wrapper instead (see ci/README.md "沙箱自举").
set -euo pipefail

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
GENERATOR="${GENERATOR:-Ninja}"

echo "==> [1/2] PC 端单测 (software)"
cmake -S "$REPO_ROOT/software" -B "$REPO_ROOT/software/build" -G "$GENERATOR"
cmake --build "$REPO_ROOT/software/build"
ctest --test-dir "$REPO_ROOT/software/build" --output-on-failure
"$REPO_ROOT/software/build/busmon" --info

echo "==> [2/2] 固件 host 协议测试 (firmware, C11, 无硬件)"
cmake -S "$REPO_ROOT/firmware" -B "$REPO_ROOT/firmware/build" -G "$GENERATOR" \
      -DBT_FW_TARGET_STM32=OFF
cmake --build "$REPO_ROOT/firmware/build"
ctest --test-dir "$REPO_ROOT/firmware/build" --output-on-failure

echo "==> ALL HOST TESTS PASSED"
