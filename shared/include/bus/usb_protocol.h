/*
 * usb_protocol.h - USB high-speed link protocol (single source of truth).
 *
 * Endpoint plan (v3.0 solution, section 4.4):
 *   EP1 IN  Bulk      device -> host : captured frame stream (16 frames/packet)
 *   EP2 OUT Bulk      host  -> device: commands / tx requests
 *   EP3 IN  Interrupt device -> host : events / error notifications
 *
 * Batch mode: 16 frames per bulk packet, MMAP zero-copy + double buffering.
 * Protocol versioned via BT_USB_PROTOCOL_VERSION.
 *
 * Consumed by:
 *   - firmware/src/protocol/usb_protocol_handler.c
 *   - software/src/protocol/usb/usb_frame_codec.cpp
 */
#ifndef BT_SHARED_USB_PROTOCOL_H
#define BT_SHARED_USB_PROTOCOL_H

#include "bus/bus_types.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define BT_USB_PROTOCOL_VERSION 1U
#define BT_USB_MAGIC            0x5442U  /* 'BT' */

/* Endpoints */
#define BT_EP_DATA_IN  0x81U
#define BT_EP_CMD_OUT  0x02U
#define BT_EP_EVT_IN   0x83U

/* Frames per bulk data packet (batch parsing, anti interrupt-storm). */
#define BT_USB_FRAMES_PER_PACKET 16U

/* Max payload of a command packet. */
#define BT_USB_CMD_PAYLOAD_MAX 256U

#if defined(_MSC_VER)
#  pragma pack(push, 1)
#  define BT_PACKED
#else
#  define BT_PACKED __attribute__((packed))
#endif

/* ---- Command codes (host -> device) ---- */
typedef enum {
    BT_CMD_NONE = 0,
    BT_CMD_START_CAPTURE = 0x01,
    BT_CMD_STOP_CAPTURE  = 0x02,
    BT_CMD_SET_FILTER    = 0x03,
    BT_CMD_SEND_FRAME    = 0x04,
    BT_CMD_SET_SCHEDULE  = 0x05,
    BT_CMD_SYNC_TIMESTAMP= 0x06,  /* PPS / sync frame alignment */
    BT_CMD_GET_DEVICE_INFO = 0x10,
    BT_CMD_DFU_BEGIN     = 0x20,
    BT_CMD_DFU_DATA      = 0x21,
    BT_CMD_DFU_END       = 0x22,
    BT_CMD_MAX
} bt_usb_cmd_t;

typedef enum {
    BT_EVT_NONE = 0,
    BT_EVT_CAPTURE_OVERRUN = 0x01,
    BT_EVT_DEVICE_ERROR    = 0x02,
    BT_EVT_DFU_STATUS      = 0x03,
    BT_EVT_MAX
} bt_usb_evt_t;

/* ---- Generic packet header ---- */
typedef struct BT_PACKED {
    uint16_t magic;      /* BT_USB_MAGIC                       */
    uint8_t  version;    /* BT_USB_PROTOCOL_VERSION            */
    uint8_t  kind;       /* 1=data, 2=cmd, 3=event             */
    uint16_t length;     /* payload length (excluding header)  */
    uint16_t seq;        /* sequence number, wrap allowed      */
    uint32_t reserved;
} bt_usb_header_t;

#define BT_USB_HEADER_SIZE ((uint16_t)sizeof(bt_usb_header_t))

/* ---- Data packet: N captured frames (N <= BT_USB_FRAMES_PER_PACKET) ---- */
typedef struct BT_PACKED {
    bt_bus_frame_t frames[BT_USB_FRAMES_PER_PACKET];
} bt_usb_data_payload_t;

/* ---- Command packet ---- */
typedef struct BT_PACKED {
    uint16_t cmd;        /* bt_usb_cmd_t */
    uint16_t channel;    /* target channel or 0xFFFF = all */
    uint32_t param;      /* command-specific parameter */
    uint8_t  payload[BT_USB_CMD_PAYLOAD_MAX];
} bt_usb_cmd_payload_t;

/* ---- Event packet ---- */
typedef struct BT_PACKED {
    uint16_t event;      /* bt_usb_evt_t */
    uint16_t channel;
    uint32_t param;
    uint8_t  payload[64];
} bt_usb_evt_payload_t;

/* ---- Full packet views (header + payload), contiguous in memory ---- */
typedef struct BT_PACKED {
    bt_usb_header_t      header;
    bt_usb_data_payload_t payload;
} bt_usb_data_packet_t;

typedef struct BT_PACKED {
    bt_usb_header_t     header;
    bt_usb_cmd_payload_t payload;
} bt_usb_cmd_packet_t;

typedef struct BT_PACKED {
    bt_usb_header_t     header;
    bt_usb_evt_payload_t payload;
} bt_usb_evt_packet_t;

/* Packet kinds */
#define BT_PKT_DATA 1U
#define BT_PKT_CMD  2U
#define BT_PKT_EVT  3U

/* Maximum contiguous size of any packet on the wire. */
#define BT_USB_PACKET_MAX \
    ((uint16_t)(BT_USB_HEADER_SIZE + \
     (BT_USB_FRAMES_PER_PACKET * (uint16_t)sizeof(bt_bus_frame_t))))

#if defined(_MSC_VER)
#  pragma pack(pop)
#endif

#ifdef __cplusplus
}
#endif

#endif /* BT_SHARED_USB_PROTOCOL_H */
