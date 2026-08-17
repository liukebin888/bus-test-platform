# ci/ - 持续集成

Monorepo 全仓 CI。策略：**Trunk-Based + PR 门禁**（见
docs/04-standards/git-workflow.md），每次 PR / push 到 main 运行。

> **接入真实 runner 的完整步骤与验收清单见
> [`RUNNER-ONBOARDING.md`](RUNNER-ONBOARDING.md)**（git init/remote/push、
> GitHub Connector 连接、act 本地替代、6 项验收标准）。

## 流水线（.github/workflows/ci.yml）

| Job | 内容 | 门禁 |
|-----|------|------|
| `sw-host` | PC 端编译（`g++` + `clang++` matrix）→ ctest 21 项单测 + `busmon --info` 冒烟 | 必须通过 |
| `fw-host` | 固件 host 协议测试（C11 编译 + ctest） | 必须通过 |
| `rtl-syntax` | FPGA RTL 语法检查（iverilog，全量编译 + 逐顶层 `-s`） | 必须通过 |
| `docs` | 文档链接/引用一致性（占位） | 可选 |

构建语义唯一入口 **`ci/scripts/ci-build.sh`**（workflow 与本地共用，避免双份
漂移）：`--only software` / `--only firmware` / 无参 = 全跑，编译器经
`CXX` / `CC` 环境变量注入。

`shared/include/bus/`（SSoT）任何变更触发全仓重跑——跨域接口不允许漂移。

### 2026-08-17 修复记录

1. `sw-host` 矩阵编译器 `gcc/clang` → `g++/clang++`（C 编译器名驱动 C++ 不规范）；
2. `rtl-syntax` 单文件编译 → 全量 `*.v` + 逐顶层（`top.v` 实例化 6 子模块，
   单文件必报 `Unknown module type`）；
3. workflow 内联步骤 → 统一调用 `ci-build.sh`；
4. `ci-build.sh` 增加 `win_path`（cygpath 条件转换），Windows git-bash 与
   Ubuntu 双平台可用；
5. 消警：`test_usb_protocol.c` 未初始化 `bt_bus_frame_t f` → `= {0}`
   （clang `-Wuninitialized-const-pointer`）。

## 沙箱自举（无系统编译器时的本地验证）

WorkBuddy 沙箱没有系统编译器，可通过 pip 安装的 zig 工具链验证：
`cmake + ninja + ziglang`（内含 clang）。zig 的 libc++ 头文件对 Windows
产生海量告警，故编译器走 `-Wno-everything` 包装器 `.toolcheck/zig-c++.cmd`；
CMake 对 clang 强制探测归档器，需在编译器目录放置 `llvm-ar.exe`
（转发到 `zig ar` 的包装，见 `.toolcheck/arwrap.c`）。真实 CI（Ubuntu）
使用原生 GCC/Clang，`ci-release` 预设启用 `-Wall -Wextra -Wpedantic -Werror`。

```bash
# 真实环境（推荐）
bash test-engineering/scripts/run_host_tests.sh

# 沙箱（zig 自举，见 .toolcheck/README 说明）
cmake -S software -B software/build -G Ninja \
      -DCMAKE_CXX_COMPILER=".../.toolcheck/zig-c++.cmd"
cmake --build software/build && ctest --test-dir software/build
```
