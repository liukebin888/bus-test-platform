# test-engineering/ - 测试工程（用例 / 报告 / 自动化）

面向 CAN / CAN FD / CAN XL / LIN 总线测试平台的**测试工程域**。
与 docs/03-plan/development-plan.md 的 Phase C（W29-40 测试发布）对齐，
承载验证活动与可追溯性（需求 → 用例 → 缺陷 → 发布门禁）。

## 目录结构

```
test-engineering/
├── cases/        # 测试用例（按层/域组织，REQ-xxx 追溯）
├── reports/      # 测试报告与覆盖度数据（CI 产出物）
└── scripts/      # 测试执行脚本（host 单测 / RTL 语法检查等）
```

## 分层测试策略（与五层软件架构 / FPGA+MCU 对应）

| 层 | 对象 | 手段 | 载体 |
|----|------|------|------|
| L0 core | 对象模型（BusFrame/Channel/Workspace） | C++ 单测 | `software/tests/unit/test_bus_frame.cpp` |
| L3 protocol | DBC 解析 / CAN 解码 / USB 编解码 | C++ 单测（**跨域字节级一致性**） | `test_dbc_parser.cpp` `test_usb_codec.cpp` |
| L2 data | SPSC 环形管线 | C++ 单测（溢出/回绕） | `test_pipeline.cpp` |
| L4 engine | RBS 周期调度 | C++ 单测（catch-up 安全） | `test_rbs_scheduler.cpp` |
| FW host | USB 协议编解码（C11，无硬件） | C 单测 | `firmware/tests/host/test_usb_protocol.c` |
| FPGA | RTL IP（时间戳/过滤/注入） | Verilog testbench | `hardware/fpga/tb/*.v` |
| 系统 | 整机（硬件 + 固件 + PC） | 台架测试（Phase C） | 待建 |

> 关键约束：PC 端与固件端共享 `shared/include/bus/` 头文件（SSoT），
> 两侧的 USB 协议测试各自验证字节级编码，防止跨域接口漂移。

## 用例追溯约定

- 用例编号：`TC-<域>-<序号>`，如 `TC-PROTO-001`（USB 数据包往返）。
- 每个用例头部注明需求来源（REQ-BUS-002 等，见 `docs/01-requirements/`）。
- Phase C 门禁要求：核心域用例 100% 通过，回归通过率 ≥ 99%（见 milestones.md）。

## 本地执行

```bash
# PC 端单测（真实环境：任意 C++17 工具链）
cmake -S software -B software/build
cmake --build software/build
ctest --test-dir software/build --output-on-failure

# 固件 host 协议测试（C11，无需 ARM 工具链）
cmake -S firmware -B firmware/build -DBT_FW_TARGET_STM32=OFF
cmake --build firmware/build
ctest --test-dir firmware/build --output-on-failure

# 或一键脚本
bash test-engineering/scripts/run_host_tests.sh
```
