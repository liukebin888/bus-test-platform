# 硬件规格（Hardware Specification）

> 来源：《汽车总线测试平台技术解决方案（集成版 v3.0）》第 4 章（4.2 / 4.3 / 4.4 / 4.5）。
> 本文为工程落地规格，器件选型以本表为准。

## 1. 总体架构

对标 Vector VN1630A / VN8900 系列：**FPGA + MCU 双核**。
FPGA 承担时间关键任务（打戳/过滤/注入/采样/调度/同步）；MCU 承担协议处理、调度表执行与 USB 传输。

## 2. 器件选型表

| 功能模块 | 推荐器件 | 关键参数 | 升级要点 |
|---|---|---|---|
| 主控 MCU | STM32H750VBT6 | 480MHz Cortex-M7，双 FDCAN（每组 2 通道） | 代际跃升自 STM32F407 |
| FPGA 协处理器 | Artix-7 XC7A35T / 国产 EG4S20 | 20-33K LUT, 225 I/O | 时间关键任务全硬件化 |
| USB PHY | USB3300 (ULPI) | USB 2.0 HS 480Mbps | 吞吐提升 40 倍（12→480Mbps） |
| CAN 收发器 ×4 | TJA1463 (CAN SIC XL) | CAN 2.0/FD/XL, 20Mbit/s | 兼容 CAN XL 物理层 |
| LIN 收发器 ×2 | TJA1027T/20 | 20kbps, 内部上拉 1kΩ | 独立 LIN 电源引脚 |
| 隔离 | ISO1042 ×4 + ISO7742 ×2 | 5kVrms, 双向 | 每通道独立隔离 |
| 时钟 | TCXO 10MHz ±10ppm | 低温漂 | 时间戳长期稳定性 |
| 同步接口 | PPS 输入 + 级联同步帧 | <1µs 跨设备偏差 | 对标 MagiSync |
| 物理层采样 | FPGA 内置 ADC | 1GS/s 8bit | 内嵌示波器功能 |

## 3. FPGA IP 功能划分（hardware/fpga/src/）

| IP 模块 | 文件 | 功能 | 性能指标 |
|---|---|---|---|
| 时间戳引擎 | `timestamp_engine.v` | 所有帧统一打戳（TCXO 参考） | 100ns 分辨率 |
| 硬件过滤器 | `hw_filter.v` | ID 掩码/范围/类型过滤 | 4 通道并行，零 CPU |
| 错误帧注入器 | `error_injector.v` | 位/CRC/ACK/填充/毛刺注入 | 逐位级精度 |
| 物理层采样 | `phys_sampler.v` | 总线电平采样 + 波形缓冲 | 1GS/s，256KB 环形 |
| 发送调度器 | `tx_scheduler.v` | 周期/事件/突发发送时序 | 抖动 <100µs |
| PPS 同步 | `pps_sync.v` | 跨设备时钟对齐 | <1µs |

## 4. USB 高速链路

- USB3300 ULPI PHY + STM32H750 OTG HS 控制器，480Mbps。
- 端点：EP1 Bulk IN（数据流 1024B 多包）/ EP2 Bulk OUT（命令/发送）/ EP3 Interrupt IN（事件/错误）。
- 带宽核算：4×CAN FD 满负载 ≈ 4MB/s 净荷，480Mbps 链路理论 60MB/s，**余量 2.3 倍**。
- 批量解析 16 帧/包，MMAP 零拷贝 + 双缓冲。
- 固件升级：DFU 模式 + 双 Bank OTA，掉电安全。

## 5. 规格对标（vs VN1630A）

| 规格 | Vector VN1630A | 本平台 v3.0 | 结论 |
|---|---|---|---|
| 架构 | FPGA/ASIC + MCU | Artix-7 + STM32H750 | 同构 |
| 通道 | 4×CAN/FD（可扩展） | 8 通道（4×CAN FD/XL + 2×LIN + 预留） | 超越 |
| USB | 2.0 HS / 3.0 | USB 2.0 HS 480Mbps | 持平 |
| 时间戳 | 100ns | 100ns（FPGA） | 持平 |
| 聚合吞吐 | ≥30k msg/s | ≥50k msg/s | 超越 |
| 错误注入 | 部分型号 | 全通道位级注入 | 超越 |
| 物理层采样 | 需外接示波器 | 内嵌 1GS/s 采样 | 超越 |
| 多设备同步 | 硬件同步卡 | PPS + 级联 4 台=32 通道 | 持平 |
| 价格 | 数万元/通道级 | 约 1/10 成本 | 成本优势 |

## 6. 待办（Phase A 前置）

- [ ] 原理图评审：USB 差分对阻抗、隔离分区、电源树
- [ ] PCB：4 层以上、高速走线规则、EMC 预测试计划
- [ ] FPGA 国产化备选 EG4S20 工具链兼容性评估
- [ ] 时钟树与 PPS 输入接口确认
