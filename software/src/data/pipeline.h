// pipeline.h - Lock-free SPSC ring buffer of BusFrame (L2 data path)
//
// v3.0 section 8: zero-copy pipeline between the USB HAL producer and the
// trace/statistics consumers. Single-producer/single-consumer; push never
// blocks - on overflow the frame is dropped and counted.
#pragma once

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <vector>

#include "core/bus_frame.h"

namespace bt {

class Pipeline {
public:
    explicit Pipeline(std::size_t capacity = 4096);
    ~Pipeline() = default;

    Pipeline(const Pipeline&) = delete;
    Pipeline& operator=(const Pipeline&) = delete;

    // Producer side. Returns false (and increments dropped()) when full.
    bool push(const BusFrame& frame);

    // Consumer side. Returns false when empty.
    bool pop(BusFrame* out);

    std::size_t size() const;
    std::size_t capacity() const { return capacity_; }
    bool empty() const;

    void reset();

    // Frames dropped due to overflow (diagnostics, REQ-DIAG-001).
    uint64_t dropped() const {
        return dropped_.load(std::memory_order_relaxed);
    }

private:
    std::vector<BusFrame> buf_;
    std::size_t capacity_;
    std::atomic<std::size_t> head_{0};  // next read index
    std::atomic<std::size_t> tail_{0};  // next write index
    std::atomic<uint64_t> dropped_{0};
};

}  // namespace bt
