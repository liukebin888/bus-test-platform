// test_rbs_scheduler.cpp - L4 engine: restbus periodic scheduling
#include <vector>

#include "engine/rbs/rbs_scheduler.h"
#include "test_framework.h"

using namespace bt;

BT_TEST(rbs_periodic_emission) {
    RbsScheduler s;

    RbsTask a;
    a.msg_id = 0x100;
    a.channel = 0;
    a.period_us = 100;
    a.dlc = 8;
    s.add_task(a);

    RbsTask b;
    b.msg_id = 0x200;
    b.channel = 1;
    b.period_us = 200;
    b.dlc = 4;
    s.add_task(b);
    BT_CHECK_EQ(s.task_count(), 2U);

    std::vector<BusFrame> out;

    // t=1000 anchors both tasks; no emission yet.
    BT_CHECK_EQ(s.tick(1000, &out), 0U);
    BT_CHECK(out.empty());

    // t=1100: task A emits its first frame.
    BT_CHECK_EQ(s.tick(1100, &out), 1U);
    BT_CHECK_EQ(out.size(), 1U);
    BT_CHECK_EQ(out[0].id, 0x100U);
    BT_CHECK_EQ(out[0].channel, 0U);
    BT_CHECK_EQ(out[0].dir, BT_DIR_TX);
    BT_CHECK_EQ(out[0].dlc, 8U);

    // t=1200: A second frame + B first frame.
    BT_CHECK_EQ(s.tick(1200, &out), 2U);
    BT_CHECK_EQ(out.size(), 3U);
    BT_CHECK_EQ(out[1].id, 0x100U);
    BT_CHECK_EQ(out[2].id, 0x200U);
    BT_CHECK_EQ(out[2].channel, 1U);

    // t=1250: nothing due.
    BT_CHECK_EQ(s.tick(1250, &out), 0U);
    BT_CHECK_EQ(out.size(), 3U);

    // Catch-up at t=1500: A at 1300/1400/1500 (3) + B at 1400 (1).
    BT_CHECK_EQ(s.tick(1500, &out), 4U);
    BT_CHECK_EQ(out.size(), 7U);

    return true;
}

BT_TEST(rbs_remove_and_disable) {
    RbsScheduler s;
    RbsTask a;
    a.msg_id = 0x100;
    a.channel = 0;
    a.period_us = 100;
    s.add_task(a);

    RbsTask b;
    b.msg_id = 0x100;
    b.channel = 1;
    b.period_us = 100;
    b.enabled = false;
    s.add_task(b);

    BT_CHECK_EQ(s.task_count(), 2U);
    BT_CHECK(s.remove_task(0x100U, 1U));   // remove the disabled one
    BT_CHECK(!s.remove_task(0x100U, 1U));  // already gone
    BT_CHECK_EQ(s.task_count(), 1U);

    std::vector<BusFrame> out;
    BT_CHECK_EQ(s.tick(1000, &out), 0U);
    BT_CHECK_EQ(s.tick(1100, &out), 1U);
    BT_CHECK_EQ(out[0].channel, 0U);

    s.clear_tasks();
    BT_CHECK_EQ(s.task_count(), 0U);
    return true;
}

BT_TEST(rbs_zero_period_is_ignored) {
    RbsScheduler s;
    RbsTask t;
    t.msg_id = 0x300;
    t.period_us = 0;  // invalid; must not spin
    s.add_task(t);

    std::vector<BusFrame> out;
    BT_CHECK_EQ(s.tick(1000, &out), 0U);
    BT_CHECK_EQ(s.tick(uint64_t{1} << 40, &out), 0U);  // far future safe
    BT_CHECK(out.empty());
    return true;
}
