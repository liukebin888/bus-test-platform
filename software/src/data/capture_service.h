// capture_service.h - Producer thread: USB HAL -> Pipeline (L2, WP-PC2)
//
// v3.0 section 8 thread model: one capture thread polls the device with a
// bounded batch and pushes into the SPSC pipeline; consumers pop from their
// own thread(s). start()/stop() are idempotent and join the worker so the
// service can be restarted deterministically.
#pragma once

#include <atomic>
#include <cstdint>
#include <thread>

#include "data/pipeline.h"
#include "hal/usb/usb_device.h"

namespace bt {

class CaptureService {
public:
    // Batch size per poll cycle; matches USB_FRAMES_PER_PACKET headroom.
    static constexpr std::size_t kBatchFrames = 256;

    CaptureService(UsbDevice& dev, Pipeline& pipe, uint32_t poll_timeout_ms = 1);
    ~CaptureService();

    CaptureService(const CaptureService&) = delete;
    CaptureService& operator=(const CaptureService&) = delete;

    // Spawns the worker thread. Returns false if already running or the
    // device is not open.
    bool start();
    // Requests stop and joins the worker. Safe to call when not running.
    void stop();

    bool running() const { return running_.load(std::memory_order_acquire); }

    // Frames handed to the pipeline (accepted + dropped).
    uint64_t offered() const { return offered_.load(std::memory_order_relaxed); }
    // Frames the pipeline accepted.
    uint64_t pushed() const { return pushed_.load(std::memory_order_relaxed); }
    // Poll cycles completed (throughput diagnostics).
    uint64_t poll_cycles() const {
        return poll_cycles_.load(std::memory_order_relaxed);
    }

private:
    void run();

    UsbDevice& dev_;
    Pipeline& pipe_;
    uint32_t poll_timeout_ms_;
    std::thread worker_;
    std::atomic<bool> running_{false};
    std::atomic<uint64_t> offered_{0};
    std::atomic<uint64_t> pushed_{0};
    std::atomic<uint64_t> poll_cycles_{0};
};

}  // namespace bt
