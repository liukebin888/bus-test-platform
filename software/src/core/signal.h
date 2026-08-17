// signal.h - DBC signal definition with raw/physical conversion
#pragma once

#include <cstdint>
#include <string>

namespace bt {

// One DBC signal (subset of the Vector DBC grammar used by DbcParser).
struct Signal {
    std::string name;
    uint8_t start_bit = 0;   // bit index in the message (Intel: LSB-first)
    uint8_t length = 1;
    double factor = 1.0;
    double offset = 0.0;
    double min_value = 0.0;
    double max_value = 0.0;
    std::string unit;
    bool is_signed = false;
    bool little_endian = true;   // Intel byte order (true) / Motorola (false)

    // Physical = raw * factor + offset.
    // For signed signals the caller must pass a sign-extended raw value
    // (see CanDecoder::sign_extend); we reinterpret it as int64 so the
    // scaling of negative values is correct.
    double raw_to_physical(uint64_t raw) const {
        if (is_signed) {
            return static_cast<double>(static_cast<int64_t>(raw)) * factor +
                   offset;
        }
        return static_cast<double>(raw) * factor + offset;
    }

    // Raw = round((physical - offset) / factor)
    uint64_t physical_to_raw(double physical) const {
        double raw = (physical - offset) / factor;
        return raw <= 0.0 ? 0ULL : static_cast<uint64_t>(raw + 0.5);
    }
};

}  // namespace bt
