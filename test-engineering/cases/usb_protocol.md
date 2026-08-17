# USB 链路协议用例（TC-PROTO-xxx）

需求来源：REQ-USB-001（USB 高速链路）、REQ-DIAG-001（诊断与统计）。
单一事实来源：`shared/include/bus/usb_protocol.h`（固件与 PC 共同引用）。

## TC-PROTO-001 命令包往返（host → device）

- 输入：`BT_CMD_SEND_FRAME`，channel=2，param=0x12345678，payload={AA BB CC DD}，seq=7
- 期望：总长 = 12(头) + 8(命令头) + 4(payload) = 24 字节；
  header.magic=0x5442、version=1、kind=2、length=12、seq=7（小端）
- 执行：PC `test_usb_codec.cpp::usb_codec_cmd_roundtrip`
        FW `test_usb_protocol.c`（同布局断言）

## TC-PROTO-002 数据包往返（device → host）

- 输入：3 帧（CAN / CAN FD / LIN 各一），帧结构体原样拷贝
- 期望：length = 3 × sizeof(bt_bus_frame_t)；解码后逐字节 memcmp 一致
- 执行：`usb_codec_data_roundtrip`

## TC-PROTO-003 事件包往返

- 输入：`BT_EVT_CAPTURE_OVERRUN`，param=12345，seq=9
- 期望：kind=3、length=8、evt/channel/param 解码一致
- 执行：`usb_codec_evt_roundtrip`

## TC-PROTO-004 非法输入拒绝

- 魔数错误 / 版本不符 / 长度越界 / kind 不匹配 → 解析返回失败
- 帧数超 `BT_USB_FRAMES_PER_PACKET`、payload 超 256 → 编码返回 0
- 执行：`usb_codec_rejects_garbage` `usb_codec_rejects_oversize`

## TC-PROTO-005 固件 host 协议测试（C11）

- 头布局（sizeof == 12）、data/cmd/evt 打包字节精确、无效输入拒绝
- 执行：`firmware/tests/host/test_usb_protocol.c`（host 目标，无需硬件）
