# 系统架构说明

> 来源：《汽车总线测试平台技术解决方案（集成版 v3.0）》第 3、6 章。
> 本文描述工程落地视角的架构视图；详细设计见上游方案与各模块代码。

## 1. 三域全景

```
┌─────────────────────────────────────────────────────────────┐
│ ① 硬件平台  FPGA + MCU 双核（对标 VN1630A 架构）              │
│   ┌──────────┐ ┌───────────────────┐ ┌────────────────────┐ │
│   │ CAN×4/LIN×2│ │ Artix-7 FPGA      │ │ STM32H750 主控     │ │
│   │ TJA1463   │ │ 100ns打戳/过滤/    │ │ FDCAN/LIN 驱动     │ │
│   │ TJA1027   │ │ 注入/1GS/s采样/PPS │ │ USB3300 HS 480Mbps │ │
│   └──────────┘ └───────────────────┘ └────────────────────┘ │
├─────────────────────────────────────────────────────────────┤
│ ② 软件平台  PC 应用（Qt6 + C++17 + Python3.11）五层架构       │
│   L5 UI 层 → L4 业务逻辑层 → L3 协议栈层 → L2 数据处理层 → L1 HAL 层 │
│   核心引擎：RBS 仿真 / Python DSL+CAPL 兼容 / UDS 诊断栈 / 测试引擎 │
├─────────────────────────────────────────────────────────────┤
│ ③ 测试管理闭环  对标 vTESTstudio + TFS + ALM                 │
│   产品中心 → 测试计划 → Test Runner → 结果管理 → 缺陷中心      │
└─────────────────────────────────────────────────────────────┘
```

## 2. 软件五层架构（software/ 目录对应）

| 层 | 目录 | 职责 | 关键技术 |
|---|---|---|---|
| L5 UI | `software/src/ui/` | 四区布局主界面、各视图 | Qt6 Widgets（可选构建） |
| L4 业务逻辑 | `software/src/engine/` | RBS 仿真、脚本宿主、Test Runner、诊断栈 | 确定性事件循环 |
| L3 协议栈 | `software/src/protocol/` | CAN/LIN 编解码、DBC/LDF 解析、ISO-TP、UDS | 纯 C++17 |
| L2 数据处理 | `software/src/data/` | 零拷贝管线、环形缓冲、日志落盘 | MMAP + 双缓冲 |
| L1 HAL | `software/src/hal/` | USB 设备抽象、设备枚举、传输 | WinUSB / 抽象接口 |

依赖方向：**上层依赖下层，禁止反向**。`core/` 为跨层共享对象模型（BusFrame 等）。

## 3. 核心数据结构（BusFrame，L2→L5 贯通）

见 `software/src/core/bus_frame.h` 与 `shared/include/bus/bus_types.h`：

```
struct BusFrame {
  uint64_t timestamp;   // 100ns 分辨率
  BusType  type;        // CAN / CANFD / CANXL / LIN
  uint8_t  channel;
  Direction dir;        // RX / TX
  uint32_t id;          // 标准/扩展 ID（含标志位）
  bool     extended;
  bool     fd;          // CAN FD 标志
  uint8_t  dlc;
  uint8_t  data[64];    // CAN XL 最大 64B
  FrameStatus status;   // OK / ERROR / OVERRUN
};
```

## 4. 数据流程（8 阶段，端到端 < 1ms）

```
总线 → FPGA(打戳/过滤) → MCU(协议解析) → USB HS → HAL I/O 线程
     → 解码线程池(批量 16 帧) → L3 协议栈 → L4 业务引擎 → L5 UI(16ms 批刷新)
```

## 5. 线程模型

| 线程 | 职责 | 优先级 |
|---|---|---|
| HAL I/O 线程 | USB 异步读写、设备枚举 | 最高 |
| 解码线程池 | 协议解码 + DBC 解析（批量 16 帧） | 高 |
| 仿真/脚本线程 | RBS 事件循环、Python GIL 隔离 | 高 |
| 测试执行线程 | Test Runner 调度、断言看门狗 | 高 |
| UI 主线程 | 渲染 + 交互（16ms 批量刷新） | 普通 |
| 日志线程 | MMAP 异步落盘 | 低 |

## 6. 与上游方案的一致性

- 版本指标统一：时间戳 100ns、延迟 < 1ms、覆盖度 92%。
- 保留差距（明确不做）：FlexRay/MOST 全总线族、HIL 硬件在环、CAPL 存量生态（提供转换工具链）。
