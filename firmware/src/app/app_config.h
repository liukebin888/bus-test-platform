/*
 * app_config.h - Application-level configuration (buffers, endpoints)
 */
#ifndef BT_FW_APP_CONFIG_H
#define BT_FW_APP_CONFIG_H

#include <stdint.h>
#include "bus/usb_protocol.h"   /* shared protocol definitions */

/* ---- Streaming buffers ---- */
#define APP_RX_PACKET_COUNT      64U    /* 64 x 16-frame bulk packets      */
#define APP_CMD_QUEUE_DEPTH      16U
#define APP_EVT_QUEUE_DEPTH      16U

/* ---- Scheduling ---- */
#define APP_SCHED_BASE_TICK_US   100U   /* 100 ns tick equivalent          */
#define APP_TX_JITTER_TARGET_US  100U

/* ---- Firmware identity (reported by BT_CMD_GET_DEVICE_INFO) ---- */
#define APP_FW_NAME              "bt-fw-h750"
#define APP_FW_VERSION_MAJOR     0U
#define APP_FW_VERSION_MINOR     1U
#define APP_FW_VERSION_PATCH     0U

/* ---- Build-time static assertion on shared struct size ---- */
typedef char bt_usb_header_size_check[
    (BT_USB_HEADER_SIZE == 12U) ? 1 : -1];

#endif /* BT_FW_APP_CONFIG_H */
