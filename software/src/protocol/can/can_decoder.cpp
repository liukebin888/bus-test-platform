// can_decoder.cpp - Decode a bus frame against a DBC message template
#include "protocol/can/can_decoder.h"

namespace bt {

uint64_t CanDecoder::extract_raw(const Signal& sig, const uint8_t* data,
                                 uint8_t dlc) {
    uint64_t v = 0;
    if (data == nullptr) {
        return 0;
    }
    const uint8_t len = (sig.length > 64U) ? 64U : sig.length;
    if (len == 0U) {
        return 0;
    }

    if (sig.little_endian) {
        // Intel: start_bit is the LSB; bits advance linearly through bytes.
        for (uint8_t i = 0U; i < len; ++i) {
            const unsigned bit = static_cast<unsigned>(sig.start_bit) + i;
            const unsigned byte = bit / 8U;
            if (byte >= dlc) {
                break;  // frame too short; remaining bits read as 0
            }
            const unsigned b = bit % 8U;
            if ((data[byte] >> b) & 1U) {
                v |= (uint64_t{1} << i);
            }
        }
    } else {
        // Motorola: start_bit is the MSB position. Within a byte the bits
        // run 7..0, then continue at bit 7 of the next byte.
        int pos = sig.start_bit;
        for (uint8_t i = 0U; i < len; ++i) {
            const int byte = pos / 8;
            if (byte >= static_cast<int>(dlc)) {
                break;  // frame too short
            }
            const int b = 7 - (pos % 8);
            v = (v << 1) | static_cast<uint64_t>((data[byte] >> b) & 1U);
            ++pos;
        }
    }
    return v;
}

uint64_t CanDecoder::sign_extend(uint64_t raw, uint8_t length) {
    if (length == 0U || length >= 64U) {
        return raw;
    }
    const uint64_t sign = uint64_t{1} << (length - 1U);
    if (raw & sign) {
        raw |= ~((uint64_t{1} << length) - 1U);
    }
    return raw;
}

bool CanDecoder::decode_frame(const DbcMessage& msg, const BusFrame& frame,
                              std::vector<SignalValue>* out) {
    if (out == nullptr) {
        return false;
    }
    out->clear();
    for (const Signal& s : msg.signals) {
        uint64_t raw = extract_raw(s, frame.data, frame.dlc);
        if (s.is_signed) {
            raw = sign_extend(raw, s.length);
        }
        SignalValue sv;
        sv.message_name = msg.name;
        sv.signal_name = s.name;
        sv.raw = raw;
        sv.physical = s.raw_to_physical(raw);
        out->push_back(sv);
    }
    return true;
}

}  // namespace bt
