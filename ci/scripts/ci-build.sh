#!/usr/bin/env bash
# ci-build.sh - CI host build entry (used by the workflow; local-safe).
#
# Builds software (host, C++17) and/or firmware host tests (C11) with a real
# toolchain. This script is the SINGLE source of build semantics for the CI
# workflow (ci/.github/workflows/ci.yml) and for local reproduction:
#
#   bash ci/scripts/ci-build.sh                 # software + firmware host
#   bash ci/scripts/ci-build.sh --only software # PC 端矩阵（配合 CXX 环境变量）
#   bash ci/scripts/ci-build.sh --only firmware # 固件 host 协议测试
#   CXX=clang++ bash ci/scripts/ci-build.sh --only software
#
# 本地无系统编译器时（如 WorkBuddy 沙箱），可借 zig 自举：
#   CXX="zig c++ -Wno-everything -Wno-error" \
#   CC="zig cc -Wno-everything -Wno-error" \
#   bash ci/scripts/ci-build.sh --only software
set -euo pipefail

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"  # POSIX（bash 操作）
# git-bash(Windows) 下 cmake.exe 是原生程序，必须用 Windows 风格路径；
# Ubuntu 无 cygpath，原样返回 POSIX 路径。保证脚本双平台可用。
win_path() {
  if command -v cygpath >/dev/null 2>&1; then cygpath -w "$1"; else echo "$1"; fi
}
REPO_ROOT_WIN="$(win_path "$REPO_ROOT")"  # Windows（cmake 参数）

# --- 参数解析 ---------------------------------------------------------------
ONLY=""
while [ $# -gt 0 ]; do
  case "$1" in
    --only) ONLY="$2"; shift 2 ;;
    *) echo "unknown arg: $1" >&2; exit 2 ;;
  esac
done

CXX="${CXX:-g++}"   # 矩阵/工具链通过环境变量注入
CC="${CC:-gcc}"

build_dir() { # $1=subdir  ->  Windows 风格 build 目录（cmake 用）
  echo "$REPO_ROOT_WIN/$1/build"
}

run_software() {
  local bd
  bd="$(build_dir software)"
  echo "==> software (host, C++17, CXX=$CXX)"
  cmake -S "$REPO_ROOT_WIN/software" -B "$bd" -G Ninja \
        -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_COMPILER="$CXX"
  cmake --build "$bd" --parallel
  ctest --test-dir "$bd" --output-on-failure
  "$REPO_ROOT/software/build/busmon" --info   # POSIX 路径（bash 执行）
}

run_firmware() {
  local bd
  bd="$(build_dir firmware)"
  echo "==> firmware host tests (C11, CC=$CC)"
  cmake -S "$REPO_ROOT_WIN/firmware" -B "$bd" -G Ninja \
        -DBT_FW_TARGET_STM32=OFF -DCMAKE_C_COMPILER="$CC"
  cmake --build "$bd" --parallel
  ctest --test-dir "$bd" --output-on-failure
}

case "$ONLY" in
  software) run_software ;;
  firmware) run_firmware ;;
  ""|all)   run_software; run_firmware ;;
  *) echo "unknown --only value: $ONLY (software|firmware|all)" >&2; exit 2 ;;
esac

echo "==> OK"
