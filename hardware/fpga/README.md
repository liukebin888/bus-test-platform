# fpga/ — FPGA 协处理器（Artix-7 / EG4S20）

对应方案 4.3「FPGA 协处理器功能划分」，6 个 IP 模块 + 顶层：

| 模块 | 文件 | 说明 |
|---|---|---|
| 顶层 | `src/top.v` | 模块互联 + MCU 接口 |
| 时间戳引擎 | `src/timestamp_engine.v` | TCXO 基准 100ns 打戳 |
| 硬件过滤器 | `src/hw_filter.v` | ID 掩码/范围/类型过滤，4 通道并行 |
| 错误帧注入器 | `src/error_injector.v` | 位/CRC/ACK/填充/毛刺注入 |
| 物理层采样 | `src/phys_sampler.v` | 1GS/s 采样 + 256KB 环形缓冲 |
| 发送调度器 | `src/tx_scheduler.v` | 周期/事件/突发发送时序 |
| PPS 同步 | `src/pps_sync.v` | 跨设备时钟对齐 |

## 仿真

```bash
iverilog -g2012 -o tb.out src/*.v tb/tb_*.v && vvp tb.out
```

CI 提供 verilator lint（见 `ci/github/workflows/fpga-lint.yml`）。

## 工程生成（Vivado 2023.x）

```bash
vivado -mode batch -source vivado/create_project.tcl
```

## 设计纪律

- 同步逻辑统一 `posedge clk`；组合逻辑禁止 latch。
- CDC 路径标注 `// CDC:` 并过同步器/异步 FIFO。
- 每个模块必须配套 testbench（`tb/`），仿真通过后方可上板。
- 国产 EG4S20 需同步评估约束与 IP 兼容性。
