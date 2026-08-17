#!/usr/bin/env bash
# local_build.sh - 本地一键构建（支持无系统编译器环境）
#
# 优先使用 zig 自举工具链（WorkBuddy 沙箱无系统 GCC/Clang 时）：
#   - 编译器   : zig c++（-Wno-everything 包装，规避 zig libc++ 头文件告警）
#   - 归档器   : arwrap/llvm-ar 包装（转发 `zig ar`；CMake 对 clang 强制探测
#                llvm-ar，故必须与编译器同目录放置）
# 真实环境（有系统工具链）直接调用 cmake+ninja。
#
# 用法: bash tools/local_build.sh [software|firmware|all]
set -euo pipefail

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
TARGET="${1:-all}"

# --- 工具定位 ------------------------------------------------------------
find_cmake() {
  if command -v cmake >/dev/null 2>&1; then
    command -v cmake
  elif [ -x "$PYTHON_ENV/Scripts/cmake.exe" ]; then
    echo "$PYTHON_ENV/Scripts/cmake.exe"
  else
    echo "cmake"
  fi
}
CMAKE="${BT_CMAKE:-$(find_cmake)}"
PYTHON_ENV="${PYTHON_ENV:-$HOME/.workbuddy/binaries/python/envs/default}"

ZIG_CC=""
if command -v zig >/dev/null 2>&1; then
  ZIG_CC="$(command -v zig) c++ -Wno-everything -Wno-error"
fi

build_host() { # $1=src  $2=build  $3=extra cmake args...
  local src="$1" build="$2"
  shift 2
  if [ -n "$ZIG_CC" ]; then
    "$CMAKE" -S "$src" -B "$build" -G Ninja \
      -DCMAKE_CXX_COMPILER="$(command -v zig)" \
      -DCMAKE_CXX_FLAGS="-Wno-everything -Wno-error" "$@"
  else
    "$CMAKE" -S "$src" -B "$build" -G Ninja "$@"
  fi
  "$CMAKE" --build "$build"
  ctest --test-dir "$build" --output-on-failure
}

if [ "$TARGET" = software ] || [ "$TARGET" = all ]; then
  echo "==> [software] PC 端构建 + 单测"
  build_host "$REPO_ROOT/software" "$REPO_ROOT/software/build"
  "$REPO_ROOT/software/build/busmon" --info
fi

if [ "$TARGET" = firmware ] || [ "$TARGET" = all ]; then
  echo "==> [firmware] host 协议测试 (C11, 无硬件)"
  build_host "$REPO_ROOT/firmware" "$REPO_ROOT/firmware/build" \
    -DBT_FW_TARGET_STM32=OFF
fi

echo "==> OK"
