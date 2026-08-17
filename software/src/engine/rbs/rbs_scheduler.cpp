// rbs_scheduler.cpp - Restbus simulation scheduler (L4 engine)
#include "engine/rbs/rbs_scheduler.h"

namespace bt {

void RbsScheduler::reset() {
    for (RbsTask& t : tasks_) {
        t.next_tick_us = 0;
    }
}

void RbsScheduler::add_task(const RbsTask& task) {
    tasks_.push_back(task);
}

bool RbsScheduler::remove_task(uint32_t msg_id, uint8_t channel) {
    for (auto it = tasks_.begin(); it != tasks_.end(); ++it) {
        if (it->msg_id == msg_id && it->channel == channel) {
            tasks_.erase(it);
            return true;
        }
    }
    return false;
}

void RbsScheduler::clear_tasks() { tasks_.clear(); }

std::size_t RbsScheduler::tick(uint64_t now_us, std::vector<BusFrame>* out) {
    if (out == nullptr) {
        return 0;
    }
    std::size_t emitted = 0;
    for (RbsTask& t : tasks_) {
        if (!t.enabled || t.period_us == 0U) {
            continue;
        }
        if (t.next_tick_us == 0U) {
            // First tick anchors the schedule: first frame goes out one
            // period after the engine starts (no burst at t=0).
            t.next_tick_us = now_us + t.period_us;
            continue;
        }
        // Catch-up safe: advance by whole periods even after a stall.
        while (t.next_tick_us <= now_us) {
            BusFrame f =
                make_frame(t.bus_type, t.channel, BT_DIR_TX, t.msg_id,
                           /*extended=*/false,
                           /*fd=*/(t.bus_type == BT_BUS_CANFD ||
                                   t.bus_type == BT_BUS_CANXL),
                           t.dlc, t.data,
                           /*timestamp_ns100=*/now_us * 10U);
            out->push_back(f);
            ++emitted;
            t.next_tick_us += t.period_us;
        }
    }
    return emitted;
}

}  // namespace bt
