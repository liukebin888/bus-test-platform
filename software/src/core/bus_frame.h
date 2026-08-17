// bus_frame.h - Canonical bus frame helpers (L0 object model)
//
// The frame STRUCT itself lives in shared/include/bus/bus_types.h
// (single source of truth shared with firmware). This header only adds
// C++ convenience helpers on top of it.
#pragma once

#include <string>

#include "bus/bus_types.h"

namespace bt {

// Canonical frame type = shared struct (do NOT redefine the layout here).
using BusFrame = bt_bus_frame_t;

// Factory helper with defaults.
BusFrame make_frame(bt_bus_type_t type, uint8_t channel,
                    bt_bus_direction_t dir, uint32_t id, bool extended,
                    bool fd, uint8_t dlc, const uint8_t* data,
                    uint64_t timestamp_ns100 = 0);

// timestamp_ns100 (100 ns ticks) -> nanoseconds
uint64_t timestamp_ns(const BusFrame& f);

// One-line human readable trace, e.g.:
//   [100000.000 us] CANFD ch0 RX id=0x123 ext=0 dlc=8 01 02 03 04 ...
std::string frame_to_string(const BusFrame& f);

}  // namespace bt
