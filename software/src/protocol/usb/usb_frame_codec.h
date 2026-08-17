// usb_frame_codec.h - Host-side USB link protocol codec
//
// Mirrors firmware/src/protocol/usb_protocol_handler.c byte-for-byte so
// both sides of the link agree on the wire format (little-endian).
// Single source of truth for the *layout* remains shared/include/bus/
// usb_protocol.h; this class implements encode/decode on top of it.
#pragma once

#include <cstddef>
#include <cstdint>

#include "bus/bus_types.h"
#include "bus/usb_protocol.h"
#include "core/bus_frame.h"

namespace bt {

class UsbFrameCodec {
public:
    // ---- encode (host -> device or device -> host for tests) ----
    // All return the total packet bytes written, or 0 on invalid input.

    // Command packet (BT_PKT_CMD): cmd(2) channel(2) param(4) payload.
    static std::size_t encode_cmd(uint8_t* buf, std::size_t cap,
                                  uint16_t cmd, uint16_t channel,
                                  uint32_t param,
                                  const uint8_t* payload = nullptr,
                                  std::size_t payload_len = 0,
                                  uint16_t seq = 0);

    // Data packet (BT_PKT_DATA): up to BT_USB_FRAMES_PER_PACKET frames.
    static std::size_t encode_data(uint8_t* buf, std::size_t cap,
                                   const BusFrame* frames,
                                   std::size_t frame_count,
                                   uint16_t seq = 0);

    // Event packet (BT_PKT_EVT): evt(2) channel(2) param(4) payload.
    static std::size_t encode_evt(uint8_t* buf, std::size_t cap,
                                  uint16_t evt, uint16_t channel,
                                  uint32_t param,
                                  const uint8_t* payload = nullptr,
                                  std::size_t payload_len = 0,
                                  uint16_t seq = 0);

    // ---- decode (device -> host) ----

    // Validates magic / version / length; returns false on any mismatch.
    static bool parse_header(const uint8_t* buf, std::size_t len,
                             uint16_t* kind, uint16_t* length,
                             uint16_t* seq);

    // Decodes a data packet into *out. Returns frame count (0 on error).
    static std::size_t decode_data(const uint8_t* buf, std::size_t len,
                                   BusFrame* out, std::size_t max_frames,
                                   uint16_t* seq = nullptr);

    // Decodes a command packet; returns false on malformed input.
    static bool decode_cmd(const uint8_t* buf, std::size_t len,
                           uint16_t* cmd, uint16_t* channel,
                           uint32_t* param);

    // Decodes an event packet; returns false on malformed input.
    static bool decode_evt(const uint8_t* buf, std::size_t len,
                           uint16_t* evt, uint16_t* channel,
                           uint32_t* param);
};

}  // namespace bt
