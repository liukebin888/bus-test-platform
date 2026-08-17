// usb_device_null.cpp - Null/loopback USB device (self-test, no hardware)
#include "hal/usb/usb_device_null.h"

namespace bt {

bool NullUsbDevice::open() {
    open_ = true;
    return true;
}

void NullUsbDevice::close() { open_ = false; }

bool NullUsbDevice::is_open() const { return open_; }

std::string NullUsbDevice::name() const { return "null://loopback"; }

bool NullUsbDevice::send_command(uint16_t cmd, uint16_t channel,
                                 uint32_t param, const uint8_t* payload,
                                 std::size_t payload_len) {
    (void)cmd;
    (void)channel;
    (void)param;
    (void)payload;
    (void)payload_len;
    if (!open_) {
        return false;
    }
    ++cmd_count_;
    return true;  // accept and drop
}

std::size_t NullUsbDevice::read_frames(BusFrame* out, std::size_t max_frames,
                                       uint32_t timeout_ms) {
    (void)out;
    (void)max_frames;
    (void)timeout_ms;
    return 0;
}

uint64_t NullUsbDevice::error_count() const { return 0; }

}  // namespace bt
