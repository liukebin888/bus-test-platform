// test_bus_frame.cpp - L0 object model: BusFrame helpers
#include <cstring>
#include <string>

#include "core/bus_frame.h"
#include "test_framework.h"

using namespace bt;

BT_TEST(bus_frame_make_and_timestamp) {
    uint8_t d[8] = {1, 2, 3, 4, 5, 6, 7, 8};
    BusFrame f =
        make_frame(BT_BUS_CAN, 0, BT_DIR_RX, 0x123, false, false, 8, d, 1000);
    BT_CHECK_EQ(f.type, BT_BUS_CAN);
    BT_CHECK_EQ(f.channel, 0U);
    BT_CHECK_EQ(f.dir, BT_DIR_RX);
    BT_CHECK_EQ(f.id, 0x123U);
    BT_CHECK_EQ(f.dlc, 8U);
    BT_CHECK(std::memcmp(f.data, d, 8) == 0);
    BT_CHECK_EQ(f.status, BT_FRAME_OK);
    // 1000 ticks * 100 ns = 100000 ns
    BT_CHECK_EQ(timestamp_ns(f), 100000ULL);
    return true;
}

BT_TEST(bus_frame_null_data_is_safe) {
    BusFrame f =
        make_frame(BT_BUS_CANFD, 1, BT_DIR_TX, 0x200, true, true, 8, nullptr, 5);
    BT_CHECK_EQ(f.fd, 1U);
    BT_CHECK_EQ(f.extended, 1U);
    BT_CHECK_EQ(f.data[0], 0U);  // zero-initialized, no crash
    return true;
}

BT_TEST(bus_frame_to_string) {
    uint8_t d[4] = {0x01, 0x02, 0x03, 0x04};
    BusFrame f =
        make_frame(BT_BUS_CANFD, 1, BT_DIR_TX, 0x456, true, true, 4, d, 500);
    const std::string s = frame_to_string(f);
    BT_CHECK(!s.empty());
    BT_CHECK(s.find("CANFD") != std::string::npos);
    BT_CHECK(s.find("ch1") != std::string::npos);
    BT_CHECK(s.find("TX") != std::string::npos);
    BT_CHECK(s.find("0x456") != std::string::npos);
    return true;
}
