# 汽车总线测试平台 (Bus Test Platform)

面向 **CAN / CAN FD / CAN XL / LIN** 总线的监控、仿真、诊断、自动化测试与工程管理一体化平台。
本仓库是《汽车总线测试平台技术解决方案（集成版 v3.0）》的落地工程，覆盖硬件、FPGA、固件、PC 软件与测试工程五个专业域。

> **上游设计文档**：`docs/reference/汽车总线测试平台技术解决方案-集成版.html`（v3.0 单一事实来源，12 章）
> 综合覆盖度 ≈ 92% vs Vector CANoe 12，40 周开发计划，10-15 人团队配置。

## 平台定位

- **对标对象**：Vector CANoe 12 + vTESTstudio + TFS + Test Report Viewer；Polarion / TestRail / Jira
- **差异化支柱**：① Python DSL + CAPL 兼容脚本 ② 8 通道 CAN+LIN 集成硬件 ③ 中文本地化生态 ④ 总线现场原生追溯（失败→缺陷一键建单）
- **核心指标**：聚合吞吐 ≥ 50k msg/s ｜ 时间戳 100ns ｜ 端到端延迟 < 1ms ｜ 发送周期抖动 < 100µs ｜ 多设备同步 < 1µs

## 仓库结构（Monorepo）

```
bus-test-platform/
├── docs/                  # 工程文档：需求/架构/计划/规范/ADR/上游方案
├── shared/                # 跨子项目共享定义（总线类型、USB 协议帧格式 = 单一事实来源）
├── hardware/              # 硬件域：板卡规格/BOM/原理图PCB占位 + FPGA RTL IP（6 个模块）
├── firmware/              # 固件域：STM32H750 驱动/协议/USB/DFU（host-sim 可编译自测）
├── software/              # PC 软件域：Qt6 + C++17 五层架构 + Python SDK
├── test-engineering/      # 测试工程域：用例库/公共步骤库/报告模板
├── ci/                    # CI/CD：GitHub Actions workflows + 构建脚本
└── tools/                 # 本地工具链与辅助脚本
```

## 快速开始

### PC 软件（software/）

```bash
# 方式一：常规环境（需要 CMake ≥3.24 与任一 C++17 编译器）
cmake -S software -B software/build -G Ninja
cmake --build software/build
ctest --test-dir software/build --output-on-failure

# 方式二：本仓库自带本地工具链脚本（无编译器环境用 zig/clang 验证）
bash tools/local_build.sh

# 运行最小自检 CLI
./software/build/busmon --info
```

- UI 层（Qt6）默认不参与构建：`-DBT_ENABLE_UI=ON` 且需本机装有 Qt6 Widgets。
- Python DSL 宿主：`-DBT_ENABLE_PYTHON=ON`（需 Python3 开发库）。
- 依赖清单见 `software/vcpkg.json`（CI 环境用 vcpkg 安装 Qt6）。

### 固件（firmware/）

```bash
# host 侧协议单测（无 ARM 工具链也可跑）
cmake -S firmware -B firmware/build-host -G Ninja
cmake --build firmware/build-host && firmware/build-host/fw_protocol_test

# 真实 STM32H750 交叉编译（需 arm-none-eabi-gcc）
cmake -S firmware -B firmware/build-stm32 -G Ninja \
  -DCMAKE_TOOLCHAIN_FILE=cmake/arm-none-eabi-toolchain.cmake -DBT_FW_TARGET_STM32=ON
```

### FPGA（hardware/fpga/）

Vivado 2023.x：`vivado -mode batch -source vivado/create_project.tcl`
RTL 仿真：`iverilog -o tb.out src/*.v tb/tb_*.v && vvp tb.out`（或使用 CI 中的 verilator lint）。

## 里程碑总览

| 里程碑 | 时段 | 验收标准 |
|---|---|---|
| M1 硬件平台 | W01-14 | 8 通道板 + 吞吐/时间戳基准达标 |
| M2 仿真诊断 Beta | W15-28 | RBS + DSL + 协议族 + 诊断功能完整 |
| M3 v3.0 正式发布 | W29-40 | 测试闭环 + 兼容层 + 认证 + 覆盖度 92% |

详见 `docs/03-plan/development-plan.md` 与 `docs/03-plan/milestones.md`。

## 文档索引

| 路径 | 内容 |
|---|---|
| `docs/01-requirements/requirements.md` | 需求基线（REQ-xxx 编号） |
| `docs/02-architecture/architecture.md` | 三域全景 + 五层软件架构 + 线程模型 |
| `docs/03-plan/development-plan.md` | 40 周 WBS（3 Phase + 测试闭环插装） |
| `docs/04-standards/` | 编码规范 / Git 工作流 / 评审清单 |
| `docs/05-adr/` | 架构决策记录（ADR-001~004） |
| `docs/reference/` | 上游技术解决方案（集成版 v3.0 HTML） |

## 开发状态

- [x] 工程骨架（Monorepo 结构 + 构建系统 + CI 资产）— 2026-08-17
- [x] PC 端骨架可编译：5 静态库 + `busmon` CLI + 21 项单元测试全绿
      （zig/clang 自举验证，Ubuntu CI 预设 `-Wall -Wextra -Wpedantic -Werror`）
- [x] PC 端 WP-PC1~3（Phase A 首波）：SimUsbDevice 流量生成器 +
      CaptureService 采集线程 + `busmon --bench` M1 基准工具；28 项单测全绿；
      主机侧门禁实测：管线 2.88M msg/s、端到端 48µs、codec 6.6M msg/s — 2026-08-17
- [ ] Phase A：硬件平台 + 固件 + 基础软件重构（W01-14）
- [ ] Phase B：仿真引擎 + 脚本 + 协议族 + 诊断（W15-28）
- [ ] Phase C：测试框架 + 兼容层 + 认证发布（W29-40）
