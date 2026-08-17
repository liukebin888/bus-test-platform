#!/usr/bin/env bash
# run_host_tests.sh - 全部 host 测试（SW + FW）统一入口。
#
# 薄封装：把执行委托给 ci/scripts/ci-build.sh（构建语义单一来源，
# 支持 CXX/CC 环境变量注入、Windows git-bash 与 Ubuntu 双平台）。
#
#   bash test-engineering/scripts/run_host_tests.sh          # SW + FW
#   bash test-engineering/scripts/run_host_tests.sh --only software
#   bash test-engineering/scripts/run_host_tests.sh --only firmware
set -euo pipefail

HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
exec bash "$HERE/../../ci/scripts/ci-build.sh" "$@"
