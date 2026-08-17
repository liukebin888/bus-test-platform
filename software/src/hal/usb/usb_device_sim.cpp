// usb_device_sim.cpp - Configurable traffic generator behind the HAL (L1)
#include "hal/usb/usb_device_sim.h"

#include <chrono>

namespace bt {

SimUsbDevice::SimUsbDevice(const SimConfig& cfg) : cfg_(cfg) {
    if (cfg_.rate_frames_per_sec == 0U) {
        cfg_.rate_frames_per_sec = 1U;
    }
    if (cfg_.dlc > BT_BUS_MAX_PAYLOAD) {
        cfg_.dlc = BT_BUS_MAX_PAYLOAD;
    }
    if (cfg_.channel_mask == 0U) {
        cfg_.channel_mask = 1U;
    }
    prng_state_ = cfg_.seed ? cfg_.seed : 1U;
}

uint64_t SimUsbDevice::now_ns() const {
    const auto tp = std::chrono::steady_clock::now().time_since_epoch();
    return static_cast<uint64_t>(
        std::chrono::duration_cast<std::chrono::nanoseconds>(tp).count());
}

bool SimUsbDevice::open() {
    opened_at_ns_ = now_ns();
    generated_.store(0U, std::memory_order_relaxed);
    id_offset_ = 0U;
    open_.store(true, std::memory_order_release);
    return true;
}

void SimUsbDevice::close() { open_.store(false, std::memory_order_release); }

bool SimUsbDevice::is_open() const {
    return open_.load(std::memory_order_acquire);
}

std::string SimUsbDevice::name() const { return "sim://generator"; }

bool SimUsbDevice::send_command(uint16_t cmd, uint16_t channel, uint32_t param,
                                const uint8_t* payload,
                                std::size_t payload_len) {
    (void)cmd;
    (void)channel;
    (void)param;
    (void)payload;
    (void)payload_len;
    return is_open();  // accept and drop, like the null device
}

std::size_t SimUsbDevice::read_frames(BusFrame* out, std::size_t max_frames,
                                      uint32_t timeout_ms) {
    (void)timeout_ms;  // frames are already "on the wire"; never blocks
    if (!is_open() || out == nullptr || max_frames == 0U) {
        return 0;
    }

    const uint64_t elapsed_ns = now_ns() - opened_at_ns_;
    // Frames due by now, clamped to what the caller can absorb.
    uint64_t due =
        (elapsed_ns / 1000000000ULL) * cfg_.rate_frames_per_sec +
        ((elapsed_ns % 1000000000ULL) * cfg_.rate_frames_per_sec) /
            1000000000ULL;
    const uint64_t already = generated_.load(std::memory_order_relaxed);
    if (due <= already) {
        return 0;
    }
    uint64_t count = due - already;
    if (count > static_cast<uint64_t>(max_frames)) {
        count = static_cast<uint64_t>(max_frames);
    }

    // Enabled channel list (precomputed walk order stays cheap).
    uint8_t channels[BT_BUS_MAX_CHANNELS];
    std::size_t nch = 0;
    for (uint32_t i = 0; i < BT_BUS_MAX_CHANNELS; ++i) {
        if ((cfg_.channel_mask & (1U << i)) != 0U) {
            channels[nch++] = static_cast<uint8_t>(i);
        }
    }

    const uint64_t ts_ns = now_ns();
    for (uint64_t i = 0; i < count; ++i) {
        BusFrame f{};
        f.timestamp_ns100 = ts_ns / 100ULL;
        f.type = cfg_.bus_type;
        f.channel = channels[(already + i) % nch];
        f.dir = BT_DIR_RX;
        f.id = cfg_.id_base + id_offset_;
        f.extended = cfg_.extended ? 1U : 0U;
        f.fd = cfg_.fd ? 1U : 0U;
        f.dlc = cfg_.dlc;
        for (uint8_t b = 0; b < cfg_.dlc; ++b) {
            prng_state_ ^= prng_state_ << 13;
            prng_state_ ^= prng_state_ >> 7;
            prng_state_ ^= prng_state_ << 17;
            f.data[b] = static_cast<uint8_t>(prng_state_ >> 56);
        }
        f.status = BT_FRAME_OK;
        out[i] = f;
        ++id_offset_;
    }
    generated_.fetch_add(count, std::memory_order_relaxed);
    return static_cast<std::size_t>(count);
}

uint64_t SimUsbDevice::error_count() const { return 0; }

}  // namespace bt
