// usb_device_sim.h - Configurable traffic generator behind the HAL (L1)
//
// Phase A WP-PC1: produces frames at a target rate so that the pipeline,
// capture service and benchmark tooling can be exercised without hardware
// (v3.0 section 8 "Null Device" strategy). Frames become "due" based on
// wall-clock elapsed time since open(), mimicking hardware arrival cadence.
#pragma once

#include <atomic>
#include <cstdint>

#include "hal/usb/usb_device.h"

namespace bt {

struct SimConfig {
    // Aggregate frame rate across all enabled channels (msg/s).
    uint32_t rate_frames_per_sec = 50000;  // aligned with the M1 target

    // Bit i enables channel i (0..BT_BUS_MAX_CHANNELS-1).
    uint32_t channel_mask = 0x0FU;  // CAN FD ch0..3 by default

    bt_bus_type_t bus_type = BT_BUS_CANFD;

    uint8_t dlc = 8;             // payload bytes per frame (<= BT_BUS_MAX_PAYLOAD)
    uint32_t id_base = 0x100U;   // IDs increment from here, wrapping
    bool extended = false;
    bool fd = true;

    // Payload fill: xorshift64 PRNG seeded per device (deterministic runs).
    uint64_t seed = 0x9E3779B97F4A7C15ULL;
};

class SimUsbDevice : public UsbDevice {
public:
    explicit SimUsbDevice(const SimConfig& cfg = SimConfig());

    bool open() override;
    void close() override;
    bool is_open() const override;
    std::string name() const override;

    bool send_command(uint16_t cmd, uint16_t channel, uint32_t param,
                      const uint8_t* payload = nullptr,
                      std::size_t payload_len = 0) override;

    // Generates every frame that has come "due" since the previous call
    // (bounded by max_frames). Timestamps come from the steady clock.
    std::size_t read_frames(BusFrame* out, std::size_t max_frames,
                            uint32_t timeout_ms) override;

    uint64_t error_count() const override;

    // Diagnostics.
    uint64_t generated() const { return generated_.load(std::memory_order_relaxed); }
    const SimConfig& config() const { return cfg_; }

private:
    uint64_t now_ns() const;

    SimConfig cfg_;
    std::atomic<bool> open_{false};
    std::atomic<uint64_t> generated_{0};
    uint64_t opened_at_ns_ = 0;   // steady-clock ns at open()
    uint64_t prng_state_ = 0;
    uint32_t id_offset_ = 0;
};

}  // namespace bt
