/*
 * usb_protocol_handler.h - USB link protocol codec (device side)
 *
 * Pure logic, no hardware dependency: fully testable on the host
 * (firmware/tests/host/test_usb_protocol.c). Encodes/decodes the shared
 * wire format defined in shared/include/bus/usb_protocol.h.
 */
#ifndef BT_FW_PROTOCOL_USB_PROTOCOL_HANDLER_H
#define BT_FW_PROTOCOL_USB_PROTOCOL_HANDLER_H

#include <stddef.h>
#include <stdint.h>

#include "bus/bus_types.h"
#include "bus/usb_protocol.h"

/* Build a data packet with 'frame_count' frames (1..BT_USB_FRAMES_PER_PACKET).
 * Returns total packet length on success, -1 on invalid input. */
int usb_protocol_build_data(uint8_t *buf, size_t cap,
                            const bt_bus_frame_t *frames,
                            uint8_t frame_count, uint16_t seq);

/* Build a command packet. Returns total length or -1. */
int usb_protocol_build_cmd(uint8_t *buf, size_t cap,
                           bt_usb_cmd_t cmd, uint16_t channel,
                           uint32_t param,
                           const uint8_t *payload, uint16_t plen,
                           uint16_t seq);

/* Build an event packet. Returns total length or -1. */
int usb_protocol_build_evt(uint8_t *buf, size_t cap,
                           bt_usb_evt_t evt, uint16_t channel,
                           uint32_t param,
                           const uint8_t *payload, uint16_t plen,
                           uint16_t seq);

/* Validate header; on success fills *hdr and sets *payload/_len to the
 * in-buffer payload region. Returns 0 or -1. */
int usb_protocol_parse(const uint8_t *buf, size_t len,
                       bt_usb_header_t *hdr,
                       const uint8_t **payload, size_t *payload_len);

/* Device-side command dispatcher (called from main loop).
 * Returns 0 when handled, -1 on unknown/invalid command. */
int usb_protocol_handle_cmd(const uint8_t *pkt, size_t len);

#endif /* BT_FW_PROTOCOL_USB_PROTOCOL_HANDLER_H */
