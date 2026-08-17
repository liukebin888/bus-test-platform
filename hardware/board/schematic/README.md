# schematic/ — 原理图工程（占位）

原理图使用 **KiCad 8**（开源，便于版本管理）或 **Altium Designer**（团队习惯）。
本目录将在 Phase A W01-04 填充实际工程文件。

## 建议页组织

| 页 | 内容 |
|---|---|
| P1 | 电源树（5V → 3.3V/1.8V/1.0V/隔离侧） |
| P2 | STM32H750 最小系统 + 时钟 + DFU Flash |
| P3 | USB3300 ULPI PHY + USB-C 连接器 |
| P4 | Artix-7 FPGA + 配置 + PPS 接口 |
| P5-P6 | CAN×4 通道（TJA1463 + ISO1042 + 保护） |
| P7 | LIN×2 通道（TJA1027 + ISO7742 + 保护） |
| P8 | 连接器 / 测试点 / ESD / 装配说明 |

## 评审清单（对应 docs/04-standards/review-checklist.md）

- [ ] USB3300 ULPI 时序（CLK 60MHz）与布线阻抗
- [ ] 隔离分区爬电距离（ISO1042 5kVrms）
- [ ] TCXO 供电去耦与地平面分割
- [ ] PPS 输入 ESD/滤波
- [ ] 双 Bank DFU 外部 Flash 连接
