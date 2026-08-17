# ADR-003：FPGA + MCU 双核硬件架构

- 状态：**已接受**（2026-08-17）
- 提出人：硬件架构师

## 背景

对标 Vector VN1630A（内部 FPGA/ASIC + MCU 同构架构）。时间关键任务（100ns 打戳、硬件过滤、
错误注入、1GS/s 采样、发送调度、PPS 同步）无法由 MCU 软件满足。

## 决策

```
TJA1463×4 / TJA1027×2 ──> Artix-7 FPGA(XC7A35T/EG4S20) ──> STM32H750 ──> USB3300 ──> PC
   (CAN FD/XL、LIN)        时间戳/过滤/注入/采样/调度/PPS      FDCAN/LIN 驱动    ULPI PHY
                                                             协议处理/USB 传输   USB2.0 HS
```

- 主控 MCU：STM32H750VBT6（480MHz M7，双 FDCAN）。
- FPGA 协处理器：Artix-7 XC7A35T 或国产 EG4S20（20-33K LUT）。
- USB PHY：USB3300（ULPI，480Mbps），带宽余量 ≥ 2.3 倍。
- 收发器：TJA1463（CAN SIC XL，20Mbit/s）×4；TJA1027 ×2；每通道 ISO1042/ISO7742 隔离。
- 时钟：TCXO 10MHz ±10ppm；PPS + 级联同步帧（4 设备 < 1µs）。

## 备选方案

1. **纯 MCU（STM32F407 内置 USB FS）**：12Mbps 带宽不足，CAN FD 满负载必丢帧；已在上轮对标分析中否决。
2. **纯 FPGA + USB 控制器 IP**：MCU 侧灵活性差，协议栈迭代成本高；拒绝。

## 后果

- FPGA 六个 IP 模块为关键路径，需 W05-10 完成 RTL 并仿真通过（见 `hardware/fpga/src/`）。
- 固件需实现 ULPI 时序与双 Bank DFU OTA（掉电安全）。
- 国产化备选 EG4S20 需评估工具链兼容性（vivado 约束需转换）。
