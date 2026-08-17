/*
 * test_usb_protocol.c - Host-side unit tests for the USB link codec
 * (run via firmware/build-host/fw_protocol_test, no ARM toolchain needed)
 */
#include <stdio.h>
#include <string.h>

#include "bus/usb_protocol.h"
#include "usb_protocol_handler.h"

static int g_fail = 0;

#define CHECK(cond, msg)                                          \
    do {                                                          \
        if (!(cond)) {                                            \
            printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, msg);  \
            g_fail = 1;                                           \
        }                                                         \
    } while (0)

static void test_header_layout(void)
{
    CHECK(BT_USB_HEADER_SIZE == 12U, "header must be 12 bytes");
    CHECK(sizeof(bt_usb_header_t) == 12U, "header struct must pack to 12");
}

static void test_data_packet_roundtrip(void)
{
    bt_bus_frame_t frames[BT_USB_FRAMES_PER_PACKET];
    uint8_t buf[BT_USB_PACKET_MAX];
    bt_usb_header_t hdr;
    const uint8_t *payload;
    size_t plen;
    int n;
    uint8_t i;

    memset(frames, 0, sizeof(frames));
    for (i = 0U; i < BT_USB_FRAMES_PER_PACKET; ++i) {
        frames[i].timestamp_ns100 = 1000U + i;
        frames[i].type            = BT_BUS_CANFD;
        frames[i].channel         = i % 4U;
        frames[i].dir             = BT_DIR_RX;
        frames[i].id              = 0x123U + i;
        frames[i].extended        = (i % 2U) ? 1U : 0U;
        frames[i].fd              = 1U;
        frames[i].dlc             = 8U;
        frames[i].status          = BT_FRAME_OK;
        for (int b = 0; b < 8; ++b) {
            frames[i].data[b] = (uint8_t)(0xA0U + i + b);
        }
    }
    /* one CAN XL frame with full 64-byte payload */
    frames[0].type = BT_BUS_CANXL;
    frames[0].dlc  = 64U;
    for (int b = 0; b < 64; ++b) {
        frames[0].data[b] = (uint8_t)b;
    }

    n = usb_protocol_build_data(buf, sizeof(buf), frames,
                                BT_USB_FRAMES_PER_PACKET, 0x00FFU);
    CHECK(n > 0, "build_data must succeed");
    CHECK((size_t)n == BT_USB_HEADER_SIZE +
                        BT_USB_FRAMES_PER_PACKET * sizeof(bt_bus_frame_t),
          "data packet length");

    CHECK(usb_protocol_parse(buf, (size_t)n, &hdr, &payload, &plen) == 0,
          "parse must succeed");
    CHECK(hdr.magic == BT_USB_MAGIC, "magic");
    CHECK(hdr.version == BT_USB_PROTOCOL_VERSION, "version");
    CHECK(hdr.kind == BT_PKT_DATA, "kind");
    CHECK(hdr.seq == 0x00FFU, "seq");
    CHECK(plen == BT_USB_FRAMES_PER_PACKET * sizeof(bt_bus_frame_t),
          "payload length");

    /* verify frame content survives byte-exact */
    CHECK(memcmp(payload, frames, plen) == 0, "frames byte-exact");
}

static void test_cmd_packet(void)
{
    uint8_t buf[256];
    bt_usb_header_t hdr;
    const uint8_t *payload;
    size_t plen;
    uint8_t send_data[64];
    int n;

    memset(send_data, 0x5A, sizeof(send_data));
    n = usb_protocol_build_cmd(buf, sizeof(buf), BT_CMD_SEND_FRAME, 3U,
                               0xCAFEBABEU, send_data, 64U, 1U);
    CHECK(n > 0, "build_cmd");
    CHECK(usb_protocol_parse(buf, (size_t)n, &hdr, &payload, &plen) == 0,
          "parse cmd");
    CHECK(hdr.kind == BT_PKT_CMD, "cmd kind");
    CHECK(plen == 8U + 64U, "cmd payload len");
    CHECK(plen >= 8U, "cmd payload len >= 8");
    CHECK(payload[0] == (uint8_t)BT_CMD_SEND_FRAME &&
          payload[1] == ((uint8_t)(BT_CMD_SEND_FRAME >> 8)), "cmd id");
    CHECK(payload[2] == 3U && payload[3] == 0U, "channel");
    CHECK(memcmp(payload + 8, send_data, 64U) == 0, "cmd payload data");
}

static void test_evt_packet(void)
{
    uint8_t buf[128];
    bt_usb_header_t hdr;
    const uint8_t *payload;
    size_t plen;
    const uint8_t evt_info[8] = {1, 2, 3, 4, 5, 6, 7, 8};
    int n;

    n = usb_protocol_build_evt(buf, sizeof(buf), BT_EVT_CAPTURE_OVERRUN,
                               2U, 0x11223344U, evt_info, sizeof(evt_info), 7U);
    CHECK(n > 0, "build_evt");
    CHECK(usb_protocol_parse(buf, (size_t)n, &hdr, &payload, &plen) == 0,
          "parse evt");
    CHECK(hdr.kind == BT_PKT_EVT, "evt kind");
    CHECK(payload[0] == (uint8_t)BT_EVT_CAPTURE_OVERRUN, "evt id");
    CHECK(payload[2] == 2U, "evt channel");
    CHECK(memcmp(payload + 8, evt_info, sizeof(evt_info)) == 0, "evt payload");
}

static void test_invalid_inputs(void)
{
    uint8_t buf[64];
    bt_bus_frame_t f = {0};
    bt_usb_header_t hdr;
    const uint8_t *payload;
    size_t plen;

    CHECK(usb_protocol_build_data(NULL, sizeof(buf), &f, 1U, 0U) == -1,
          "null buf");
    CHECK(usb_protocol_build_data(buf, sizeof(buf), &f, 0U, 0U) == -1,
          "zero frames");
    CHECK(usb_protocol_build_data(buf, sizeof(buf), &f, 17U, 0U) == -1,
          "too many frames");
    CHECK(usb_protocol_build_data(buf, 4U, &f, 1U, 0U) == -1,
          "cap too small");
    CHECK(usb_protocol_parse(buf, 4U, &hdr, &payload, &plen) == -1,
          "short parse");

    memset(buf, 0, sizeof(buf));
    CHECK(usb_protocol_parse(buf, sizeof(buf), &hdr, &payload, &plen) == -1,
          "bad magic");
}

int main(void)
{
    test_header_layout();
    test_data_packet_roundtrip();
    test_cmd_packet();
    test_evt_packet();
    test_invalid_inputs();

    if (g_fail) {
        printf("RESULT: FAIL\n");
        return 1;
    }
    printf("RESULT: PASS (usb_protocol_handler)\n");
    return 0;
}
