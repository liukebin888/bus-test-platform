// channel.h - Bus channel object (object model: Bus -> Channel -> Node -> Signal)
#pragma once

#include <cstdint>
#include <string>

#include "bus/bus_types.h"

namespace bt {

class Channel {
public:
    Channel(uint8_t id, bt_bus_type_t type, std::string name);

    uint8_t id() const { return id_; }
    bt_bus_type_t type() const { return type_; }
    const std::string& name() const { return name_; }

    void set_enabled(bool enabled) { enabled_ = enabled; }
    bool enabled() const { return enabled_; }

    // Statistics (v3.0: Trace/Statistics views feed on these counters)
    void on_frame_rx() { ++rx_count_; ++total_frames_; }
    void on_frame_tx() { ++tx_count_; ++total_frames_; }
    void on_error() { ++error_count_; }

    uint64_t rx_count() const { return rx_count_; }
    uint64_t tx_count() const { return tx_count_; }
    uint64_t error_count() const { return error_count_; }
    uint64_t total_frames() const { return total_frames_; }

    // bus load = total_frames * 1 (simplified) / window; placeholder metric
    double bus_load_percent() const { return load_estimate_percent_; }
    void set_load_estimate(double percent) { load_estimate_percent_ = percent; }

    void reset_stats();

private:
    uint8_t id_;
    bt_bus_type_t type_;
    std::string name_;
    bool enabled_ = true;
    uint64_t rx_count_ = 0;
    uint64_t tx_count_ = 0;
    uint64_t error_count_ = 0;
    uint64_t total_frames_ = 0;
    double load_estimate_percent_ = 0.0;
};

}  // namespace bt
