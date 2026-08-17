# hardware/ — 硬件域

覆盖板卡硬件（原理图/PCB/BOM）与 FPGA 协处理器 RTL。对应方案第 4 章与 Phase A（W01-14）。

```
hardware/
├── docs/hardware-spec.md      # 硬件规格（器件选型表，源自方案 4.2）
├── board/                     # 8 通道测试板
│   ├── bom/bom.csv            # BOM 初稿（按方案 4.2 器件表）
│   ├── schematic/             # 原理图工程（占位，工具建议 KiCad/Altium）
│   └── pcb/                   # PCB 布局布线（占位）
└── fpga/                      # Artix-7 / EG4S20 FPGA
    ├── src/                   # RTL：6 个 IP 模块 + 顶层
    ├── tb/                    # testbench（iverilog 可跑）
    ├── constraints/top.xdc    # 时序/管脚约束
    ├── ip/                    # 内部 IP 核与子模块
    └── vivado/create_project.tcl  # Vivado 工程生成脚本
```

## 关键规格（速览）

- 8 通道：4×CAN FD/XL + 2×LIN + 预留
- 主控 STM32H750VBT6（480MHz M7，双 FDCAN）
- FPGA Artix-7 XC7A35T / 国产 EG4S20
- USB3300 ULPI PHY → USB 2.0 HS 480Mbps
- 收发器 TJA1463×4 + TJA1027×2，每通道 ISO1042/ISO7742 隔离
- TCXO 10MHz ±10ppm；PPS + 级联同步

## 开发阶段（Phase A）

| 周次 | 硬件活动 | FPGA 活动 |
|---|---|---|
| W01-04 | 原理图 v1.0 + 评审 | — |
| W03-06 | PCB 布局布线 v1.0 | — |
| W05-10 | 样机打样准备 | RTL 开发 + 仿真（本目录） |
| W11-14 | 整机联调 + EMC 预测试 | 上板验证 + 基准测试 |

## 交付物（方案 12.1 H 类）

原理图 / PCB / BOM / 固件源码 / FPGA IP / USB 驱动安装包 / CE·FCC 认证报告
