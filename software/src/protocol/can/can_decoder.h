// can_decoder.h - Decode a bus frame against a DBC message template
#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "core/bus_frame.h"
#include "protocol/dbc/dbc_parser.h"

namespace bt {

// One decoded signal value (L3 protocol stack output, REQ-BUS-002).
struct SignalValue {
    std::string message_name;
    std::string signal_name;
    uint64_t raw = 0;          // raw (possibly sign-extended) value
    double physical = 0.0;     // raw * factor + offset
};

class CanDecoder {
public:
    // Extracts every signal of `msg` from `frame` (CAN / CAN FD / CAN XL
    // payload; LIN frames use the same layout for decoded view).
    // Fills *out (cleared first). Returns false if *out is null.
    static bool decode_frame(const DbcMessage& msg, const BusFrame& frame,
                             std::vector<SignalValue>* out);

    // Raw bit extraction per DBC byte-order rules:
    //   Intel   (little_endian): start_bit = LSB, linear bit index
    //   Motorola (big_endian)  : start_bit = MSB position, byte-internal
    //                            order 7..0, next byte starts at bit 7
    // Bits beyond frame.dlc are read as 0 (frame too short is tolerated).
    static uint64_t extract_raw(const Signal& sig, const uint8_t* data,
                                uint8_t dlc);

    // Two's-complement sign extension for `length`-bit raw values.
    static uint64_t sign_extend(uint64_t raw, uint8_t length);
};

}  // namespace bt
