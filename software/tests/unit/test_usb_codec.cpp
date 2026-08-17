// test_usb_codec.cpp - L3 protocol: USB link codec round-trips
#include <cstring>

#include "core/bus_frame.h"
#include "protocol/usb/usb_frame_codec.h"
#include "test_framework.h"

using namespace bt;

BT_TEST(usb_codec_cmd_roundtrip) {
    uint8_t buf[512];
    const uint8_t payload[4] = {0xAA, 0xBB, 0xCC, 0xDD};
    const std::size_t n = UsbFrameCodec::encode_cmd(
        buf, sizeof(buf), static_cast<uint16_t>(BT_CMD_SEND_FRAME), 0x0002U,
        0x12345678U, payload, sizeof(payload), 7);
    BT_CHECK(n > 0U);

    uint16_t kind = 0;
    uint16_t length = 0;
    uint16_t seq = 0;
    BT_CHECK(UsbFrameCodec::parse_header(buf, n, &kind, &length, &seq));
    BT_CHECK_EQ(kind, static_cast<uint16_t>(BT_PKT_CMD));
    BT_CHECK_EQ(length, static_cast<uint16_t>(8U + sizeof(payload)));
    BT_CHECK_EQ(seq, static_cast<uint16_t>(7));

    uint16_t cmd = 0;
    uint16_t ch = 0;
    uint32_t param = 0;
    BT_CHECK(UsbFrameCodec::decode_cmd(buf, n, &cmd, &ch, &param));
    BT_CHECK_EQ(cmd, static_cast<uint16_t>(BT_CMD_SEND_FRAME));
    BT_CHECK_EQ(ch, static_cast<uint16_t>(2));
    BT_CHECK_EQ(param, 0x12345678U);
    return true;
}

BT_TEST(usb_codec_data_roundtrip) {
    uint8_t buf[4096];
    uint8_t d0[8] = {1, 2, 3, 4, 5, 6, 7, 8};
    BusFrame frames[3];
    frames[0] = make_frame(BT_BUS_CAN, 0, BT_DIR_RX, 0x100, false, false, 8,
                           d0, 10);
    frames[1] = make_frame(BT_BUS_CANFD, 1, BT_DIR_TX, 0x200, true, true, 16,
                           nullptr, 20);
    frames[2] = make_frame(BT_BUS_LIN, 2, BT_DIR_RX, 0x3C, false, false, 8,
                           nullptr, 30);

    const std::size_t n = UsbFrameCodec::encode_data(buf, sizeof(buf), frames,
                                                     3, 42);
    BT_CHECK(n > 0U);

    BusFrame out[3];
    uint16_t seq = 0;
    const std::size_t cnt = UsbFrameCodec::decode_data(buf, n, out, 3, &seq);
    BT_CHECK_EQ(cnt, 3U);
    BT_CHECK_EQ(seq, static_cast<uint16_t>(42));
    BT_CHECK(std::memcmp(&out[0], &frames[0], sizeof(BusFrame)) == 0);
    BT_CHECK(std::memcmp(&out[1], &frames[1], sizeof(BusFrame)) == 0);
    BT_CHECK(std::memcmp(&out[2], &frames[2], sizeof(BusFrame)) == 0);
    return true;
}

BT_TEST(usb_codec_evt_roundtrip) {
    uint8_t buf[256];
    const std::size_t n = UsbFrameCodec::encode_evt(
        buf, sizeof(buf), static_cast<uint16_t>(BT_EVT_CAPTURE_OVERRUN), 0U,
        12345U, nullptr, 0, 9);
    BT_CHECK(n > 0U);

    uint16_t kind = 0;
    uint16_t seq = 0;
    BT_CHECK(UsbFrameCodec::parse_header(buf, n, &kind, nullptr, &seq));
    BT_CHECK_EQ(kind, static_cast<uint16_t>(BT_PKT_EVT));
    BT_CHECK_EQ(seq, static_cast<uint16_t>(9));

    uint16_t evt = 0;
    uint16_t ch = 0;
    uint32_t param = 0;
    BT_CHECK(UsbFrameCodec::decode_evt(buf, n, &evt, &ch, &param));
    BT_CHECK_EQ(evt, static_cast<uint16_t>(BT_EVT_CAPTURE_OVERRUN));
    BT_CHECK_EQ(ch, static_cast<uint16_t>(0));
    BT_CHECK_EQ(param, 12345U);
    return true;
}

BT_TEST(usb_codec_rejects_garbage) {
    // Zero buffer: magic/version mismatch.
    uint8_t buf[128] = {0};
    uint16_t kind = 0;
    uint16_t length = 0;
    uint16_t seq = 0;
    BT_CHECK(!UsbFrameCodec::parse_header(buf, sizeof(buf), &kind, &length,
                                          &seq));
    // Truncated header.
    BT_CHECK(!UsbFrameCodec::parse_header(buf, 4, &kind, &length, &seq));
    // Corrupted magic on a valid packet.
    uint8_t d[512];
    BusFrame f = make_frame(BT_BUS_CAN, 0, BT_DIR_RX, 1U, false, false, 1, d, 0);
    const std::size_t n = UsbFrameCodec::encode_data(d, sizeof(d), &f, 1, 0);
    BT_CHECK(n > 0U);
    d[0] ^= 0xFFU;
    BusFrame out[4];
    BT_CHECK_EQ(UsbFrameCodec::decode_data(d, n, out, 4), 0U);
    // decode_cmd on a data packet must fail (kind mismatch).
    uint8_t d2[512];
    const std::size_t n2 = UsbFrameCodec::encode_data(d2, sizeof(d2), &f, 1, 0);
    BT_CHECK(n2 > 0U);
    uint16_t cmd = 0;
    uint16_t ch = 0;
    uint32_t param = 0;
    BT_CHECK(!UsbFrameCodec::decode_cmd(d2, n2, &cmd, &ch, &param));
    return true;
}

BT_TEST(usb_codec_rejects_oversize) {
    uint8_t buf[32];
    BusFrame frames[BT_USB_FRAMES_PER_PACKET];
    // Too many frames for one packet.
    BT_CHECK_EQ(UsbFrameCodec::encode_data(buf, sizeof(buf), frames,
                                           BT_USB_FRAMES_PER_PACKET + 1U, 0),
                0U);
    // Buffer too small even for one frame.
    BT_CHECK_EQ(UsbFrameCodec::encode_data(buf, sizeof(buf), frames, 1, 0), 0U);
    // Payload over the command cap.
    uint8_t big[BT_USB_CMD_PAYLOAD_MAX + 1U] = {0};
    BT_CHECK_EQ(UsbFrameCodec::encode_cmd(buf, sizeof(buf), 1, 0, 0, big,
                                          sizeof(big), 0),
                0U);
    return true;
}
