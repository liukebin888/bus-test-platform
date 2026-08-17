// test_capture_service.cpp - CaptureService unit tests (WP-PC2)
#include <chrono>
#include <thread>

#include "data/capture_service.h"
#include "hal/usb/usb_device_sim.h"
#include "test_framework.h"

using namespace bt;

BT_TEST(capture_start_opens_closed_device) {
    SimUsbDevice dev;
    dev.close();  // ensure closed: start() must auto-open the device
    Pipeline pipe(64);
    CaptureService cap(dev, pipe, 1);
    BT_CHECK(cap.start());  // auto-opens the sim device
    BT_CHECK(cap.running());
    BT_CHECK(!cap.start());  // double start rejected
    cap.stop();
    BT_CHECK(!cap.running());
    return true;
}

BT_TEST(capture_moves_frames_into_pipeline) {
    SimConfig cfg;
    cfg.rate_frames_per_sec = 20000;  // 1 frame / 50 us
    SimUsbDevice dev(cfg);
    dev.open();
    Pipeline pipe(4096);
    CaptureService cap(dev, pipe, 1);

    BT_CHECK(cap.start());
    BT_CHECK(cap.running());
    BT_CHECK(!cap.start());  // double start rejected

    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    cap.stop();
    BT_CHECK(!cap.running());
    BT_CHECK(!cap.running());

    BT_CHECK(cap.offered() > 0U);
    BT_CHECK(cap.pushed() > 0U);
    BT_CHECK(cap.poll_cycles() > 0U);
    // Consumer drains everything the pipeline accepted.
    BusFrame out;
    uint64_t drained = 0;
    while (pipe.pop(&out)) {
        ++drained;
    }
    BT_CHECK_EQ(drained, cap.pushed());
    // With a 4096-slot pipeline at 20k msg/s for 50 ms (~1000 frames)
    // nothing should have been dropped.
    BT_CHECK_EQ(pipe.dropped(), static_cast<uint64_t>(0));
    return true;
}

BT_TEST(capture_restart_is_deterministic) {
    SimConfig cfg;
    cfg.rate_frames_per_sec = 100000;
    SimUsbDevice dev(cfg);
    dev.open();
    Pipeline pipe(1024);
    CaptureService cap(dev, pipe, 1);

    BT_CHECK(cap.start());
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
    cap.stop();

    // Restart must work and counters keep accumulating (no reset).
    const uint64_t offered_before = cap.offered();
    BT_CHECK(cap.start());
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
    cap.stop();
    BT_CHECK(cap.offered() >= offered_before);
    BT_CHECK(cap.running() == false);
    return true;
}
