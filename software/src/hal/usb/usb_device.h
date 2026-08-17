// usb_device.h - L1 HAL: USB link to the device (interface)
//
// v3.0 section 8: one abstraction over the transport (libusb / WinUSB /
// null self-test). All frame traffic flows through the shared
// UsbFrameCodec; this interface only moves bytes and commands.
#pragma once

#include <cstddef>
#include <cstdint>
#include <string>

#include "core/bus_frame.h"

namespace bt {

class UsbDevice {
public:
    virtual ~UsbDevice() = default;

    virtual bool open() = 0;
    virtual void close() = 0;
    virtual bool is_open() const = 0;
    virtual std::string name() const = 0;

    // Sends one command packet (BT_PKT_CMD) to the device.
    virtual bool send_command(uint16_t cmd, uint16_t channel, uint32_t param,
                              const uint8_t* payload = nullptr,
                              std::size_t payload_len = 0) = 0;

    // Blocks up to timeout_ms for captured frames on the bulk IN endpoint.
    // Returns frames copied (0 = timeout / no data).
    virtual std::size_t read_frames(BusFrame* out, std::size_t max_frames,
                                    uint32_t timeout_ms) = 0;

    // Cumulative transport/parse errors seen (diagnostics, REQ-DIAG-001).
    virtual uint64_t error_count() const = 0;
};

}  // namespace bt
