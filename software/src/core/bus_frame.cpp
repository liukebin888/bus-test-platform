#include "core/bus_frame.h"

#include <cstdio>
#include <cstring>

namespace bt {

BusFrame make_frame(bt_bus_type_t type, uint8_t channel,
                    bt_bus_direction_t dir, uint32_t id, bool extended,
                    bool fd, uint8_t dlc, const uint8_t* data,
                    uint64_t timestamp_ns100) {
    BusFrame f{};
    f.timestamp_ns100 = timestamp_ns100;
    f.type = type;
    f.channel = channel;
    f.dir = dir;
    f.id = id;
    f.extended = extended ? 1U : 0U;
    f.fd = fd ? 1U : 0U;
    f.dlc = (dlc > BT_BUS_MAX_PAYLOAD) ? BT_BUS_MAX_PAYLOAD : dlc;
    f.status = BT_FRAME_OK;
    if (data != nullptr && f.dlc > 0U) {
        std::memcpy(f.data, data, f.dlc);
    }
    return f;
}

uint64_t timestamp_ns(const BusFrame& f) {
    // 100 ns per tick
    return f.timestamp_ns100 * 100U;
}

std::string frame_to_string(const BusFrame& f) {
    char buf[256];
    int n = std::snprintf(buf, sizeof(buf),
                          "[%llu.%03llu us] %s ch%u %s id=0x%X ext=%u fd=%u "
                          "dlc=%u st=%u",
                          static_cast<unsigned long long>(timestamp_ns(f) / 1000U),
                          static_cast<unsigned long long>(timestamp_ns(f) % 1000U),
                          bt_bus_type_str(f.type), f.channel,
                          f.dir == BT_DIR_RX ? "RX" : "TX",
                          static_cast<unsigned>(f.id), f.extended, f.fd,
                          f.dlc, static_cast<unsigned>(f.status));
    std::string s(buf, static_cast<size_t>(n));
    s += " ";
    for (uint8_t i = 0; i < f.dlc; ++i) {
        char b[4];
        std::snprintf(b, sizeof(b), "%02X ", f.data[i]);
        s += b;
    }
    return s;
}

}  // namespace bt
