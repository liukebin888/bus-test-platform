// main.cpp - busmon CLI entry point (L5 shell, no GUI dependency)
#include <cstdio>
#include <cstring>
#include <string>

#include "bus/usb_protocol.h"
#include "core/workspace.h"
#include "hal/usb/usb_device_null.h"

#ifndef BT_PROJECT_VERSION
#define BT_PROJECT_VERSION "0.0.0"
#endif

namespace {

int cmd_info() {
    bt::NullUsbDevice dev;
    if (!dev.open()) {
        std::fprintf(stderr, "error: cannot open null device\n");
        return 1;
    }
    std::printf("busmon %s - 汽车总线测试平台 PC 端骨架\n", BT_PROJECT_VERSION);
    std::printf("  device        : %s\n", dev.name().c_str());
    std::printf("  usb protocol  : v%u (magic 0x%04X)\n",
                static_cast<unsigned>(BT_USB_PROTOCOL_VERSION),
                static_cast<unsigned>(BT_USB_MAGIC));
    std::printf("  endpoints     : EP1 IN 0x%02X data | EP2 OUT 0x%02X cmd | "
                "EP3 IN 0x%02X evt\n",
                static_cast<unsigned>(BT_EP_DATA_IN),
                static_cast<unsigned>(BT_EP_CMD_OUT),
                static_cast<unsigned>(BT_EP_EVT_IN));
    std::printf("  frames/packet : %u\n",
                static_cast<unsigned>(BT_USB_FRAMES_PER_PACKET));
    std::printf("  status        : OK (null self-test device, no hardware)\n");
    return 0;
}

int cmd_demo() {
    bt::Workspace ws;
    bt::Channel* ch0 = ws.add_channel(0, BT_BUS_CANFD, "CAN FD #0");
    ws.add_channel(1, BT_BUS_CANFD, "CAN FD #1");
    ws.add_channel(2, BT_BUS_LIN, "LIN #0");
    bt::Node* n = ws.add_node("ECU_Engine", 0);
    if (ch0 == nullptr || n == nullptr) {
        std::fprintf(stderr, "error: workspace setup failed\n");
        return 1;
    }
    ch0->on_frame_rx();
    ch0->on_frame_rx();
    ch0->on_frame_tx();
    std::printf("workspace channels: %zu\n", ws.channel_count());
    for (bt::Channel* c : ws.channels()) {
        std::printf("  ch%u %s %s (rx=%llu tx=%llu)\n", c->id(),
                    c->name().c_str(), c->enabled() ? "enabled" : "disabled",
                    static_cast<unsigned long long>(c->rx_count()),
                    static_cast<unsigned long long>(c->tx_count()));
    }
    std::printf("workspace nodes   : %zu\n", ws.node_count());
    std::printf("engines           : RBS/script/test-runner skeleton (Phase B)\n");
    return 0;
}

int cmd_usage(const char* argv0) {
    const char* self = (argv0 != nullptr && argv0[0] != '\0') ? argv0 : "busmon";
    std::printf("usage: %s [--info | --demo | --help]\n", self);
    std::printf("  --info   device & protocol summary (default)\n");
    std::printf("  --demo   exercise the object model (channels/nodes)\n");
    std::printf("  --help   this message\n");
    return 0;
}

}  // namespace

int main(int argc, char** argv) {
    const char* cmd = (argc > 1) ? argv[1] : "--info";
    if (std::strcmp(cmd, "--info") == 0 || std::strcmp(cmd, "-i") == 0) {
        return cmd_info();
    }
    if (std::strcmp(cmd, "--demo") == 0 || std::strcmp(cmd, "-d") == 0) {
        return cmd_demo();
    }
    return cmd_usage((argc > 0) ? argv[0] : nullptr);
}
