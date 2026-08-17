// bench.h - M1 benchmark harness (WP-PC3)
//
// Measures the acceptance metrics of milestone M1 on the host data path
// (no hardware needed): aggregate pipeline throughput, end-to-end latency
// and USB codec round-trip cost. Results align with the M1 gate:
//   - aggregate throughput >= 50k msg/s
//   - end-to-end latency   < 1 ms
//   - zero silent loss (drop counter must account for every frame)
#pragma once

#include <cstdint>

#include "hal/usb/usb_device_sim.h"

namespace bt {

struct BenchReport {
    // --- pipeline throughput (single-thread push/pop loop) ---
    double duration_s = 0.0;
    uint64_t frames = 0;
    double pipeline_throughput = 0.0;   // msg/s

    // --- SPSC latency (push -> pop, same thread) ---
    double latency_avg_us = 0.0;
    double latency_max_us = 0.0;

    // --- overflow accounting ---
    uint64_t dropped = 0;               // must equal offered - accepted

    // --- codec round-trip (encode_data + decode_data) ---
    uint64_t codec_frames = 0;
    double codec_throughput = 0.0;      // msg/s

    // --- end-to-end (sim device -> capture thread -> consumer) ---
    uint64_t e2e_frames = 0;
    double e2e_throughput = 0.0;        // msg/s
    double e2e_latency_avg_us = 0.0;
    double e2e_latency_max_us = 0.0;
    uint64_t e2e_dropped = 0;
    double sim_rate_achieved = 0.0;     // msg/s actually generated

    // M1 gate evaluation (host-side portion).
    bool pass_throughput() const { return pipeline_throughput >= 50000.0; }
    bool pass_latency() const { return e2e_latency_avg_us < 1000.0; }
    bool pass_loss_accounting() const;
};

// Runs the full suite. duration_s bounds each stage (>= 0.1s, clamped).
BenchReport run_bench_suite(double duration_s, uint32_t sim_rate = 50000);

// Prints a human-readable report aligned with the M1 acceptance table.
void print_bench_report(const BenchReport& r);

}  // namespace bt
