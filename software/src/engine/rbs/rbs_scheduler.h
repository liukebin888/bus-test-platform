// rbs_scheduler.h - Restbus simulation scheduler (L4 engine)
//
// v3.0 section 9 (RBS 仿真): periodic transmit templates driven by a
// virtual time base. tick(now_us) emits every task whose deadline passed,
// then advances by whole periods (catch-up safe, no burst after stalls).
#pragma once

#include <cstdint>
#include <vector>

#include "core/bus_frame.h"

namespace bt {

// One periodic transmit template (user-configurable, REQ-SIM-001).
struct RbsTask {
    uint32_t msg_id = 0;
    uint8_t channel = 0;
    bt_bus_type_t bus_type = BT_BUS_CANFD;
    uint32_t period_us = 1000;  // e.g. 1000 us = 1 kHz
    uint8_t dlc = 8;
    uint8_t data[BT_BUS_MAX_PAYLOAD]{};
    bool enabled = true;
    // Scheduler-owned: next emission time in the virtual time base.
    uint64_t next_tick_us = 0;
};

class RbsScheduler {
public:
    // Re-anchors the virtual time base (first tick() after reset() starts
    // every task one period ahead, so no burst occurs at t=0).
    void reset();

    void add_task(const RbsTask& task);
    // Removes the first task with matching msg_id+channel; false if absent.
    bool remove_task(uint32_t msg_id, uint8_t channel);
    void clear_tasks();

    // Emits due frames into *out (appended). Returns number emitted.
    std::size_t tick(uint64_t now_us, std::vector<BusFrame>* out);

    std::size_t task_count() const { return tasks_.size(); }

private:
    std::vector<RbsTask> tasks_;
};

}  // namespace bt
