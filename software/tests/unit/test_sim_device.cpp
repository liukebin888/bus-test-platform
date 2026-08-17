// test_sim_device.cpp - SimUsbDevice unit tests (WP-PC1)
#include <chrono>
#include <thread>

#include "hal/usb/usb_device_sim.h"
#include "test_framework.h"

using namespace bt;

BT_TEST(sim_config_clamping) {
    SimConfig cfg;
    cfg.rate_frames_per_sec = 0;
    cfg.dlc = 200;
    cfg.channel_mask = 0;
    SimUsbDevice dev(cfg);
    BT_CHECK_EQ(dev.config().rate_frames_per_sec, 1U);
    BT_CHECK_EQ(dev.config().dlc, BT_BUS_MAX_PAYLOAD);
    BT_CHECK(dev.config().channel_mask != 0U);
    return true;
}

BT_TEST(sim_closed_device_reads_nothing) {
    SimUsbDevice dev;
    BT_CHECK(!dev.is_open());
    BusFrame out[4];
    BT_CHECK_EQ(dev.read_frames(out, 4, 1), static_cast<std::size_t>(0));
    BT_CHECK(!dev.send_command(0, 0, 0));
    BT_CHECK_EQ(dev.error_count(), static_cast<uint64_t>(0));
    return true;
}

BT_TEST(sim_generates_due_frames_over_time) {
    SimConfig cfg;
    cfg.rate_frames_per_sec = 100000;  // 1 frame per 10 us
    cfg.channel_mask = 0x5U;           // channels 0 and 2
    cfg.dlc = 16;
    SimUsbDevice dev(cfg);
    BT_CHECK(dev.open());
    BT_CHECK_EQ(dev.name(), std::string("sim://generator"));

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    BusFrame out[64];
    const std::size_t n = dev.read_frames(out, 64, 1);
    // 10 ms at 100k msg/s => ~1000 due, bounded to 64.
    BT_CHECK_EQ(n, static_cast<std::size_t>(64));
    BT_CHECK_EQ(dev.generated(), static_cast<uint64_t>(64));

    // Frame content: channel from mask, payload length, monotone ids.
    BT_CHECK(out[0].channel == 0U || out[0].channel == 2U);
    BT_CHECK_EQ(out[0].dlc, 16U);
    BT_CHECK_EQ(out[0].type, BT_BUS_CANFD);
    BT_CHECK_EQ(out[1].id - out[0].id, 1U);
    // The first read was truncated (1000 due > 64 absorbed), so the
    // backlog keeps flowing: the next read serves more backlog frames.
    const std::size_t n2 = dev.read_frames(out, 64, 1);
    BT_CHECK(n2 > 0U);
    BT_CHECK_EQ(dev.generated(), static_cast<uint64_t>(64 + n2));
    return true;
}

BT_TEST(sim_reopen_resets_counters) {
    SimConfig cfg;
    cfg.rate_frames_per_sec = 200000;
    SimUsbDevice dev(cfg);
    dev.open();
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
    BusFrame out[32];
    dev.read_frames(out, 32, 1);
    BT_CHECK(dev.generated() > 0U);
    dev.close();
    BT_CHECK(!dev.is_open());
    dev.open();
    BT_CHECK_EQ(dev.generated(), static_cast<uint64_t>(0));
    return true;
}
