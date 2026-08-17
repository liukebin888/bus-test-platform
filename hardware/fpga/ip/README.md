# fpga/ip/ — 内部 IP 与子模块

预留目录：后续将 CORDIC、双端口 RAM（采样缓冲）、异步 FIFO（CDC）等
以 Vivado IP / 国产工具 IP 形式放入本目录，并在 `docs/` 登记 IP 版本与许可证。

## 规划

| IP | 用途 | 来源 |
|---|---|---|
| 双端口 RAM 256KB | 物理层采样环形缓冲 | Xilinx Block Memory / 国产 RAM |
| 异步 FIFO | MCU 寄存器接口 CDC | Xilinx FIFO Generator / 自研 |
| 时钟管理 MMCM/PLL | 1GS/s 采样时钟生成 | Xilinx Clocking Wizard |
