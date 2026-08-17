// usb_frame_codec.cpp - Host-side USB link protocol codec
//
// Wire format (little-endian), identical to the firmware implementation:
//   header  : magic(2) version(1) kind(1) length(2) seq(2) reserved(4)
//   cmd pkt : header + cmd(2) channel(2) param(4) payload[...]
//   data pkt: header + frame structs verbatim (shared packed layout)
//   evt pkt : header + evt(2) channel(2) param(4) payload[...]
#include "protocol/usb/usb_frame_codec.h"

#include <cstring>

namespace bt {

namespace {

bool header_fill(uint8_t* buf, std::size_t cap, uint8_t kind,
                 uint16_t length, uint16_t seq) {
    if (buf == nullptr || cap < BT_USB_HEADER_SIZE) {
        return false;
    }
    buf[0] = static_cast<uint8_t>(BT_USB_MAGIC & 0xFFU);
    buf[1] = static_cast<uint8_t>((BT_USB_MAGIC >> 8) & 0xFFU);
    buf[2] = BT_USB_PROTOCOL_VERSION;
    buf[3] = kind;
    buf[4] = static_cast<uint8_t>(length & 0xFFU);
    buf[5] = static_cast<uint8_t>((length >> 8) & 0xFFU);
    buf[6] = static_cast<uint8_t>(seq & 0xFFU);
    buf[7] = static_cast<uint8_t>((seq >> 8) & 0xFFU);
    buf[8] = 0;
    buf[9] = 0;
    buf[10] = 0;
    buf[11] = 0;
    return true;
}

std::size_t encode_8_plus_payload(uint8_t* buf, std::size_t cap,
                                  uint8_t kind, uint16_t type_field,
                                  uint16_t channel, uint32_t param,
                                  const uint8_t* payload,
                                  std::size_t payload_len, uint16_t seq) {
    if (buf == nullptr || payload_len > BT_USB_CMD_PAYLOAD_MAX) {
        return 0;
    }
    const std::size_t plen = payload_len;
    const std::size_t total = BT_USB_HEADER_SIZE + 8U + plen;
    if (total > cap) {
        return 0;
    }
    if (!header_fill(buf, cap, kind,
                     static_cast<uint16_t>(8U + plen), seq)) {
        return 0;
    }
    uint8_t* p = buf + BT_USB_HEADER_SIZE;
    p[0] = static_cast<uint8_t>(type_field & 0xFFU);
    p[1] = static_cast<uint8_t>((type_field >> 8) & 0xFFU);
    p[2] = static_cast<uint8_t>(channel & 0xFFU);
    p[3] = static_cast<uint8_t>((channel >> 8) & 0xFFU);
    p[4] = static_cast<uint8_t>(param & 0xFFU);
    p[5] = static_cast<uint8_t>((param >> 8) & 0xFFU);
    p[6] = static_cast<uint8_t>((param >> 16) & 0xFFU);
    p[7] = static_cast<uint8_t>((param >> 24) & 0xFFU);
    if (plen > 0U && payload != nullptr) {
        std::memcpy(p + 8, payload, plen);
    }
    return total;
}

}  // namespace

std::size_t UsbFrameCodec::encode_cmd(uint8_t* buf, std::size_t cap,
                                      uint16_t cmd, uint16_t channel,
                                      uint32_t param,
                                      const uint8_t* payload,
                                      std::size_t payload_len,
                                      uint16_t seq) {
    return encode_8_plus_payload(buf, cap, BT_PKT_CMD, cmd, channel, param,
                                 payload, payload_len, seq);
}

std::size_t UsbFrameCodec::encode_evt(uint8_t* buf, std::size_t cap,
                                      uint16_t evt, uint16_t channel,
                                      uint32_t param,
                                      const uint8_t* payload,
                                      std::size_t payload_len,
                                      uint16_t seq) {
    return encode_8_plus_payload(buf, cap, BT_PKT_EVT, evt, channel, param,
                                 payload, payload_len, seq);
}

std::size_t UsbFrameCodec::encode_data(uint8_t* buf, std::size_t cap,
                                       const BusFrame* frames,
                                       std::size_t frame_count,
                                       uint16_t seq) {
    if (buf == nullptr || frames == nullptr || frame_count == 0U ||
        frame_count > BT_USB_FRAMES_PER_PACKET) {
        return 0;
    }
    const std::size_t frame_bytes = frame_count * sizeof(BusFrame);
    const std::size_t total = BT_USB_HEADER_SIZE + frame_bytes;
    if (total > cap) {
        return 0;
    }
    if (!header_fill(buf, cap, BT_PKT_DATA,
                     static_cast<uint16_t>(frame_bytes), seq)) {
        return 0;
    }
    std::memcpy(buf + BT_USB_HEADER_SIZE, frames, frame_bytes);
    return total;
}

bool UsbFrameCodec::parse_header(const uint8_t* buf, std::size_t len,
                                 uint16_t* kind, uint16_t* length,
                                 uint16_t* seq) {
    if (buf == nullptr || len < BT_USB_HEADER_SIZE) {
        return false;
    }
    const uint16_t magic =
        static_cast<uint16_t>(buf[0] | (static_cast<uint16_t>(buf[1]) << 8));
    const uint8_t version = buf[2];
    const uint16_t k = buf[3];
    const uint16_t l =
        static_cast<uint16_t>(buf[4] | (static_cast<uint16_t>(buf[5]) << 8));
    const uint16_t s =
        static_cast<uint16_t>(buf[6] | (static_cast<uint16_t>(buf[7]) << 8));
    if (magic != BT_USB_MAGIC || version != BT_USB_PROTOCOL_VERSION) {
        return false;
    }
    if (static_cast<std::size_t>(l) + BT_USB_HEADER_SIZE > len) {
        return false;
    }
    if (kind != nullptr) {
        *kind = k;
    }
    if (length != nullptr) {
        *length = l;
    }
    if (seq != nullptr) {
        *seq = s;
    }
    return true;
}

std::size_t UsbFrameCodec::decode_data(const uint8_t* buf, std::size_t len,
                                       BusFrame* out, std::size_t max_frames,
                                       uint16_t* seq) {
    if (buf == nullptr || out == nullptr || max_frames == 0U) {
        return 0;
    }
    uint16_t kind = 0;
    uint16_t length = 0;
    uint16_t pkt_seq = 0;
    if (!parse_header(buf, len, &kind, &length, &pkt_seq) ||
        kind != BT_PKT_DATA) {
        return 0;
    }
    if (length % sizeof(BusFrame) != 0U) {
        return 0;
    }
    const std::size_t count = length / sizeof(BusFrame);
    if (count == 0U || count > max_frames) {
        return 0;
    }
    std::memcpy(out, buf + BT_USB_HEADER_SIZE, count * sizeof(BusFrame));
    if (seq != nullptr) {
        *seq = pkt_seq;
    }
    return count;
}

bool UsbFrameCodec::decode_cmd(const uint8_t* buf, std::size_t len,
                               uint16_t* cmd, uint16_t* channel,
                               uint32_t* param) {
    if (buf == nullptr || len < BT_USB_HEADER_SIZE + 8U) {
        return false;
    }
    uint16_t kind = 0;
    uint16_t length = 0;
    if (!parse_header(buf, len, &kind, &length, nullptr) || kind != BT_PKT_CMD) {
        return false;
    }
    if (length < 8U || static_cast<std::size_t>(length) + BT_USB_HEADER_SIZE > len) {
        return false;
    }
    const uint8_t* p = buf + BT_USB_HEADER_SIZE;
    if (cmd != nullptr) {
        *cmd = static_cast<uint16_t>(p[0] | (static_cast<uint16_t>(p[1]) << 8));
    }
    if (channel != nullptr) {
        *channel =
            static_cast<uint16_t>(p[2] | (static_cast<uint16_t>(p[3]) << 8));
    }
    if (param != nullptr) {
        *param = static_cast<uint32_t>(p[4]) |
                 (static_cast<uint32_t>(p[5]) << 8) |
                 (static_cast<uint32_t>(p[6]) << 16) |
                 (static_cast<uint32_t>(p[7]) << 24);
    }
    return true;
}

bool UsbFrameCodec::decode_evt(const uint8_t* buf, std::size_t len,
                               uint16_t* evt, uint16_t* channel,
                               uint32_t* param) {
    if (buf == nullptr || len < BT_USB_HEADER_SIZE + 8U) {
        return false;
    }
    uint16_t kind = 0;
    uint16_t length = 0;
    if (!parse_header(buf, len, &kind, &length, nullptr) || kind != BT_PKT_EVT) {
        return false;
    }
    if (length < 8U || static_cast<std::size_t>(length) + BT_USB_HEADER_SIZE > len) {
        return false;
    }
    const uint8_t* p = buf + BT_USB_HEADER_SIZE;
    if (evt != nullptr) {
        *evt = static_cast<uint16_t>(p[0] | (static_cast<uint16_t>(p[1]) << 8));
    }
    if (channel != nullptr) {
        *channel =
            static_cast<uint16_t>(p[2] | (static_cast<uint16_t>(p[3]) << 8));
    }
    if (param != nullptr) {
        *param = static_cast<uint32_t>(p[4]) |
                 (static_cast<uint32_t>(p[5]) << 8) |
                 (static_cast<uint32_t>(p[6]) << 16) |
                 (static_cast<uint32_t>(p[7]) << 24);
    }
    return true;
}

}  // namespace bt
