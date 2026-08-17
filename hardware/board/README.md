# board/ — 8 通道测试板

| 项 | 内容 |
|---|---|
| 通道 | 4×CAN FD/XL + 2×LIN + 预留 |
| 主控 | STM32H750VBT6（480MHz M7） |
| FPGA | Artix-7 XC7A35T / EG4S20 |
| USB | USB3300 ULPI → USB 2.0 HS |
| 隔离 | 每通道 ISO1042 / ISO7742（5kVrms） |

## 目录

- `bom/bom.csv` — BOM 初稿（源自方案 4.2 器件表，含封装/数量/备注）
- `schematic/` — 原理图工程（占位；建议 KiCad 8 / Altium Designer）
- `pcb/` — PCB 布局布线（占位；≥4 层，高速差分/隔离分区）

## 评审要点（W01-04 原理图评审）

1. USB3300 ULPI 信号（CLK/STP/DIR/NXT/D[7:0]）阻抗与拓扑
2. CAN/LIN 收发器与隔离器供电分区（隔离电源）
3. 时钟树：TCXO → FPGA/MCU，PPS 输入 ESD 防护
4. 电源树：5V 输入 → 3.3V/1.8V/1.0V（FPGA 核）/3.3V 隔离侧
5. DFU 双 Bank 所需外部 Flash（或利用内部 128KB）

## 交付物（Phase A）

原理图 v1.0 → PCB v1.0 → 样机 3 台 → EMC 预测试 → 认证配合
