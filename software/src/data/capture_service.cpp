// capture_service.cpp - Producer thread: USB HAL -> Pipeline (L2, WP-PC2)
#include "data/capture_service.h"

namespace bt {

CaptureService::CaptureService(UsbDevice& dev, Pipeline& pipe,
                               uint32_t poll_timeout_ms)
    : dev_(dev), pipe_(pipe), poll_timeout_ms_(poll_timeout_ms) {}

CaptureService::~CaptureService() { stop(); }

bool CaptureService::start() {
    if (running_.load(std::memory_order_acquire)) {
        return false;
    }
    if (!dev_.is_open() && !dev_.open()) {
        return false;
    }
    running_.store(true, std::memory_order_release);
    worker_ = std::thread(&CaptureService::run, this);
    return true;
}

void CaptureService::stop() {
    if (!running_.exchange(false, std::memory_order_acq_rel)) {
        return;
    }
    if (worker_.joinable()) {
        worker_.join();
    }
}

void CaptureService::run() {
    BusFrame batch[kBatchFrames];
    while (running_.load(std::memory_order_acquire)) {
        const std::size_t n =
            dev_.read_frames(batch, kBatchFrames, poll_timeout_ms_);
        for (std::size_t i = 0; i < n; ++i) {
            if (pipe_.push(batch[i])) {
                pushed_.fetch_add(1U, std::memory_order_relaxed);
            }
            offered_.fetch_add(1U, std::memory_order_relaxed);
        }
        poll_cycles_.fetch_add(1U, std::memory_order_relaxed);
        if (n == 0U) {
            // Nothing due yet: yield the core instead of hot-spinning.
            std::this_thread::yield();
        }
    }
}

}  // namespace bt
