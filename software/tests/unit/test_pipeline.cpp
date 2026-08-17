// test_pipeline.cpp - L2 data path: SPSC ring buffer
#include "data/pipeline.h"
#include "test_framework.h"

using namespace bt;

BT_TEST(pipeline_fifo_order) {
    Pipeline p(4);
    BusFrame f;
    for (unsigned i = 1U; i <= 4U; ++i) {
        f = make_frame(BT_BUS_CAN, 0, BT_DIR_RX, i, false, false, 1, nullptr,
                       i);
        BT_CHECK(p.push(f));
    }
    BT_CHECK_EQ(p.size(), 4U);
    BT_CHECK_EQ(p.dropped(), 0U);
    BT_CHECK(!p.empty());

    BusFrame o;
    uint32_t expect = 1U;
    while (p.pop(&o)) {
        BT_CHECK_EQ(o.id, expect);
        BT_CHECK_EQ(o.timestamp_ns100, static_cast<uint64_t>(expect));
        ++expect;
    }
    BT_CHECK_EQ(expect, 5U);
    BT_CHECK(p.empty());
    BT_CHECK_EQ(p.size(), 0U);
    return true;
}

BT_TEST(pipeline_overflow_drops) {
    Pipeline p(2);
    BusFrame f;
    for (unsigned i = 1U; i <= 5U; ++i) {
        f = make_frame(BT_BUS_CAN, 0, BT_DIR_RX, i, false, false, 1, nullptr,
                       i);
        p.push(f);  // 3rd..5th fail silently
    }
    BT_CHECK_EQ(p.dropped(), 3U);
    BT_CHECK_EQ(p.size(), 2U);

    // Oldest two survive, in order.
    BusFrame o;
    BT_CHECK(p.pop(&o));
    BT_CHECK_EQ(o.id, 1U);
    BT_CHECK(p.pop(&o));
    BT_CHECK_EQ(o.id, 2U);
    BT_CHECK(!p.pop(&o));

    p.reset();
    BT_CHECK(p.empty());
    BT_CHECK_EQ(p.dropped(), 0U);
    return true;
}

BT_TEST(pipeline_wrap_around) {
    // Force the ring indices to wrap multiple times (capacity 3, 10 pushes).
    Pipeline p(3);
    BusFrame f;
    for (unsigned i = 1U; i <= 10U; ++i) {
        f = make_frame(BT_BUS_CAN, 0, BT_DIR_RX, i, false, false, 1, nullptr,
                       i);
        p.push(f);
        if (p.size() == 3U) {
            p.pop(&f);  // keep the ring hot so indices wrap
        }
    }
    // Drain: last 3 values pushed must come back in order.
    BusFrame o;
    uint32_t last = 0;
    while (p.pop(&o)) {
        BT_CHECK(o.id > last);
        last = o.id;
    }
    BT_CHECK_EQ(last, 10U);
    return true;
}
