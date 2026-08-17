/*
 * bus_types.h - Shared bus type definitions (single source of truth).
 *
 * Consumed by BOTH:
 *   - firmware/  (C11, STM32H750)
 *   - software/  (C++17, PC application)
 *
 * Keep this header C89/C99-compatible (no C++-only constructs outside
 * the extern "C" guard). Any change here must trigger a full-repo CI run.
 */
#ifndef BT_SHARED_BUS_TYPES_H
#define BT_SHARED_BUS_TYPES_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Bus type identifiers (v3.0 solution: CAN / CAN FD / CAN XL / LIN). */
typedef enum {
    BT_BUS_UNKNOWN = 0,
    BT_BUS_CAN,    /* CAN 2.0A/B          */
    BT_BUS_CANFD,  /* CAN FD              */
    BT_BUS_CANXL,  /* CAN XL (20 Mbit/s)  */
    BT_BUS_LIN     /* LIN 1.3/2.0/2.1     */
} bt_bus_type_t;

typedef enum {
    BT_DIR_RX = 0,
    BT_DIR_TX
} bt_bus_direction_t;

typedef enum {
    BT_FRAME_OK = 0,
    BT_FRAME_ERROR,
    BT_FRAME_OVERRUN
} bt_frame_status_t;

/* Max payload across all supported buses (CAN XL: 64 bytes). */
#define BT_BUS_MAX_PAYLOAD 64U
/* Max channels per device (4x CAN FD/XL + 2x LIN + reserved). */
#define BT_BUS_MAX_CHANNELS 8U

/*
 * Unified bus frame (mirrors BusFrame in the v3.0 solution, section 6.6).
 * timestamp_ns100: 100 ns resolution tick value (FPGA hardware stamped).
 */
typedef struct {
    uint64_t          timestamp_ns100;
    bt_bus_type_t     type;
    uint8_t           channel;   /* 0..BT_BUS_MAX_CHANNELS-1       */
    bt_bus_direction_t dir;
    uint32_t          id;        /* standard/extended ID (flags in bits) */
    uint8_t           extended;  /* 0/1 */
    uint8_t           fd;        /* CAN FD flag (0/1) */
    uint8_t           dlc;       /* data length code, bytes 0..64 */
    uint8_t           data[BT_BUS_MAX_PAYLOAD];
    bt_frame_status_t status;
} bt_bus_frame_t;

/* Human-readable helper (PC-side convenience; may be used in fw debug). */
static inline const char *bt_bus_type_str(bt_bus_type_t t) {
    switch (t) {
        case BT_BUS_CAN:   return "CAN";
        case BT_BUS_CANFD: return "CANFD";
        case BT_BUS_CANXL: return "CANXL";
        case BT_BUS_LIN:   return "LIN";
        default:           return "UNKNOWN";
    }
}

#ifdef __cplusplus
}
#endif

#endif /* BT_SHARED_BUS_TYPES_H */
