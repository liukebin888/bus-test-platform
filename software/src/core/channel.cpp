#include "core/channel.h"

namespace bt {

Channel::Channel(uint8_t id, bt_bus_type_t type, std::string name)
    : id_(id), type_(type), name_(std::move(name)) {}

void Channel::reset_stats() {
    rx_count_ = 0;
    tx_count_ = 0;
    error_count_ = 0;
    total_frames_ = 0;
    load_estimate_percent_ = 0.0;
}

}  // namespace bt
