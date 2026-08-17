/*
 * usb_protocol_handler.c - USB link protocol codec (device side)
 *
 * Endianness: little-endian wire format (both STM32 and x86-64 are LE;
 * a big-endian port would flip the *_be helpers). Sequence numbers wrap
 * and are NOT validated for gaps (host tolerates).
 */
#include "usb_protocol_handler.h"

static void put_u16(uint8_t *p, uint16_t v)
{
    p[0] = (uint8_t)(v & 0xFFU);
    p[1] = (uint8_t)((v >> 8) & 0xFFU);
}

static void put_u32(uint8_t *p, uint32_t v)
{
    p[0] = (uint8_t)(v & 0xFFU);
    p[1] = (uint8_t)((v >> 8) & 0xFFU);
    p[2] = (uint8_t)((v >> 16) & 0xFFU);
    p[3] = (uint8_t)((v >> 24) & 0xFFU);
}

static uint16_t get_u16(const uint8_t *p)
{
    return (uint16_t)(p[0] | ((uint16_t)p[1] << 8));
}

static uint32_t get_u32(const uint8_t *p)
{
    return (uint32_t)p[0] | ((uint32_t)p[1] << 8) |
           ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
}

static void header_fill(uint8_t *buf, uint8_t kind, uint16_t length,
                        uint16_t seq)
{
    put_u16(buf + 0, BT_USB_MAGIC);
    buf[2] = BT_USB_PROTOCOL_VERSION;
    buf[3] = kind;
    put_u16(buf + 4, length);
    put_u16(buf + 6, seq);
    put_u32(buf + 8, 0U); /* reserved */
}

int usb_protocol_build_data(uint8_t *buf, size_t cap,
                            const bt_bus_frame_t *frames,
                            uint8_t frame_count, uint16_t seq)
{
    const size_t frame_bytes = (size_t)frame_count * sizeof(bt_bus_frame_t);
    size_t total;

    if (buf == NULL || frames == NULL || frame_count == 0U ||
        frame_count > BT_USB_FRAMES_PER_PACKET) {
        return -1;
    }
    total = BT_USB_HEADER_SIZE + frame_bytes;
    if (total > cap) {
        return -1;
    }

    header_fill(buf, BT_PKT_DATA, (uint16_t)frame_bytes, seq);

    /* Copy frame structs verbatim (struct layout is shared & packed). */
    {
        const uint8_t *src = (const uint8_t *)frames;
        uint8_t *dst = buf + BT_USB_HEADER_SIZE;
        size_t i;
        for (i = 0U; i < frame_bytes; ++i) {
            dst[i] = src[i];
        }
    }
    return (int)total;
}

int usb_protocol_build_cmd(uint8_t *buf, size_t cap,
                           bt_usb_cmd_t cmd, uint16_t channel,
                           uint32_t param,
                           const uint8_t *payload, uint16_t plen,
                           uint16_t seq)
{
    uint8_t *p;
    size_t total;

    if (buf == NULL || plen > BT_USB_CMD_PAYLOAD_MAX) {
        return -1;
    }
    total = BT_USB_HEADER_SIZE + 8U + (size_t)plen;
    if (total > cap) {
        return -1;
    }

    header_fill(buf, BT_PKT_CMD, (uint16_t)(8U + plen), seq);
    p = buf + BT_USB_HEADER_SIZE;
    put_u16(p + 0, (uint16_t)cmd);
    put_u16(p + 2, channel);
    put_u32(p + 4, param);
    if (plen > 0U && payload != NULL) {
        size_t i;
        for (i = 0U; i < (size_t)plen; ++i) {
            p[8 + i] = payload[i];
        }
    }
    return (int)total;
}

int usb_protocol_build_evt(uint8_t *buf, size_t cap,
                           bt_usb_evt_t evt, uint16_t channel,
                           uint32_t param,
                           const uint8_t *payload, uint16_t plen,
                           uint16_t seq)
{
    uint8_t *p;
    size_t total;

    if (buf == NULL || plen > 64U) {
        return -1;
    }
    total = BT_USB_HEADER_SIZE + 8U + (size_t)plen;
    if (total > cap) {
        return -1;
    }

    header_fill(buf, BT_PKT_EVT, (uint16_t)(8U + plen), seq);
    p = buf + BT_USB_HEADER_SIZE;
    put_u16(p + 0, (uint16_t)evt);
    put_u16(p + 2, channel);
    put_u32(p + 4, param);
    if (plen > 0U && payload != NULL) {
        size_t i;
        for (i = 0U; i < (size_t)plen; ++i) {
            p[8 + i] = payload[i];
        }
    }
    return (int)total;
}

int usb_protocol_parse(const uint8_t *buf, size_t len,
                       bt_usb_header_t *hdr,
                       const uint8_t **payload, size_t *payload_len)
{
    if (buf == NULL || hdr == NULL || len < BT_USB_HEADER_SIZE) {
        return -1;
    }
    hdr->magic   = get_u16(buf + 0);
    hdr->version = buf[2];
    hdr->kind    = buf[3];
    hdr->length  = get_u16(buf + 4);
    hdr->seq     = get_u16(buf + 6);
    hdr->reserved = get_u32(buf + 8);

    if (hdr->magic != BT_USB_MAGIC ||
        hdr->version != BT_USB_PROTOCOL_VERSION) {
        return -1;
    }
    if ((size_t)hdr->length + BT_USB_HEADER_SIZE > len) {
        return -1;
    }
    if (payload != NULL) {
        *payload = buf + BT_USB_HEADER_SIZE;
    }
    if (payload_len != NULL) {
        *payload_len = (size_t)hdr->length;
    }
    return 0;
}

int usb_protocol_handle_cmd(const uint8_t *pkt, size_t len)
{
    bt_usb_header_t hdr;
    const uint8_t *payload;
    size_t plen;

    if (usb_protocol_parse(pkt, len, &hdr, &payload, &plen) != 0 ||
        hdr.kind != BT_PKT_CMD) {
        return -1;
    }
    if (plen < 8U) {
        return -1;
    }

    /* TODO(Phase A): dispatch to capture control / filter config / send
     * frame / schedule / DFU handlers. */
    return 0;
}
