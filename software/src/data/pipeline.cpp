// pipeline.cpp - Lock-free SPSC ring buffer of BusFrame (L2 data path)
#include "data/pipeline.h"

namespace bt {

Pipeline::Pipeline(std::size_t capacity)
    : buf_(capacity > 0U ? capacity : 1U), capacity_(capacity > 0U ? capacity : 1U) {}

bool Pipeline::push(const BusFrame& frame) {
    const std::size_t h = head_.load(std::memory_order_relaxed);
    const std::size_t t = tail_.load(std::memory_order_relaxed);
    if (t - h >= capacity_) {
        dropped_.fetch_add(1U, std::memory_order_relaxed);
        return false;
    }
    buf_[t % capacity_] = frame;
    tail_.store(t + 1U, std::memory_order_release);
    return true;
}

bool Pipeline::pop(BusFrame* out) {
    if (out == nullptr) {
        return false;
    }
    const std::size_t h = head_.load(std::memory_order_relaxed);
    const std::size_t t = tail_.load(std::memory_order_acquire);
    if (h == t) {
        return false;
    }
    *out = buf_[h % capacity_];
    head_.store(h + 1U, std::memory_order_release);
    return true;
}

std::size_t Pipeline::size() const {
    const std::size_t h = head_.load(std::memory_order_relaxed);
    const std::size_t t = tail_.load(std::memory_order_relaxed);
    return t - h;
}

bool Pipeline::empty() const { return size() == 0U; }

void Pipeline::reset() {
    head_.store(0U, std::memory_order_relaxed);
    tail_.store(0U, std::memory_order_relaxed);
    dropped_.store(0U, std::memory_order_relaxed);
}

}  // namespace bt
