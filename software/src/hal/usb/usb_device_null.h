// usb_device_null.h - Null/loopback USB device (self-test, no hardware)
#pragma once

#include <cstddef>
#include <cstdint>
#include <string>

#include "hal/usb/usb_device.h"

namespace bt {

// Accepts commands and drops them, never produces frames. Used by the
// busmon CLI in no-hardware mode and by HAL-contract unit tests.
class NullUsbDevice : public UsbDevice {
public:
    bool open() override;
    void close() override;
    bool is_open() const override;
    std::string name() const override;

    bool send_command(uint16_t cmd, uint16_t channel, uint32_t param,
                      const uint8_t* payload = nullptr,
                      std::size_t payload_len = 0) override;

    std::size_t read_frames(BusFrame* out, std::size_t max_frames,
                            uint32_t timeout_ms) override;

    uint64_t error_count() const override;

    // Test hook: number of accepted commands since open().
    uint64_t cmd_count() const { return cmd_count_; }

private:
    bool open_ = false;
    uint64_t cmd_count_ = 0;
};

}  // namespace bt
