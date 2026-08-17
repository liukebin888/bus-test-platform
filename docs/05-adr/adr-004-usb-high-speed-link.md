# ADR-004：USB 高速链路与帧协议设计

- 状态：**已接受**（2026-08-17）
- 提出人：嵌入式 + 软件架构师

## 背景

8 通道全速采集（4×CAN FD 满负载 ≈ 4MB/s 净荷）+ 命令/事件通道，需统一 USB 链路协议，
且固件与 PC 端必须共享同一帧格式定义（防漂移）。

## 决策

**链路**：USB3300 ULPI PHY + STM32H750 OTG HS（480Mbps）。

**端点规划**：
| 端点 | 方向 | 类型 | 用途 |
|---|---|---|---|
| EP1 IN | 设备→主机 | Bulk | 数据流（1024B 多包） |
| EP2 OUT | 主机→设备 | Bulk | 命令 / 发送请求 |
| EP3 IN | 设备→主机 | Interrupt | 事件 / 错误通知 |

**协议**：批量模式 16 帧/包；MMAP 零拷贝 + 双缓冲；DFU 模式 + 双 Bank OTA。
帧格式定义收敛于 `shared/include/bus/usb_protocol.h`（单一事实来源），
固件（`firmware/src/protocol/usb_protocol_handler.c`）与 PC 端（`software/src/protocol/usb/usb_frame_codec.cpp`）共同引用。

## 备选方案

1. **CDC ACM 虚拟串口**：吞吐与实时性不足；拒绝。
2. **每帧一个 USB 包**：小包中断风暴，吞吐不达标；拒绝（批量 16 帧/包）。

## 后果

- USB 帧格式变更必须同步修改 `shared/`，CI 全仓构建保证一致性。
- 主机侧需 WinUSB 驱动（交付物含安装包）；Linux/macOS 走 libusb 后端。
- 通过 host 侧单元测试（`firmware/tests/host/`）先行验证编解码正确性，降低联调成本。
