# firmware/ — STM32H750 固件

主控 MCU 固件：FDCAN×4 / LIN×2 驱动、USB ULPI 传输、DFU OTA，及 USB 链路协议层。
对应方案 4.4 与 Phase A（W07-12）。

## 双目标构建

| 目标 | 说明 | 命令 |
|---|---|---|
| `fw_protocol_test`（默认） | host 侧协议单测，**无需 ARM 工具链** | `cmake -S firmware -B build-host -G Ninja && cmake --build build-host` |
| `bt_fw.elf`（STM32） | 真机固件，需 arm-none-eabi-gcc | 见 README 顶层命令 |

## 目录

```
firmware/
├── config/board.h                 # 板级配置（管脚/外设/时钟）
├── src/
│   ├── main.c                     # STM32 入口（BT_FW_TARGET_STM32 编译）
│   ├── app/app_config.h           # 应用配置（端点/缓冲/命令）
│   ├── drivers/
│   │   ├── fdcan/                 # FDCAN 驱动（4 通道）
│   │   ├── lin/                   # LIN 驱动（2 通道）
│   │   ├── usb/                   # USB3300 ULPI + OTG HS
│   │   └── dfu/                   # 双 Bank DFU OTA
│   └── protocol/usb_protocol_handler.[ch]  # USB 帧协议编解码（host 可测）
└── tests/host/                    # 主机侧单元测试
```

## 设计要点

- **协议层与硬件解耦**：`usb_protocol_handler.c` 只依赖 `shared/include/bus/*.h`，
  host 单测直接覆盖编解码正确性，联调成本大幅下降。
- **ISR 纪律**：中断仅置标志/入队，禁止阻塞与动态内存（见编码规范）。
- **DFU**：双 Bank OTA + 掉电安全；升级过程 `BT_EVT_DFU_STATUS` 事件上报。
