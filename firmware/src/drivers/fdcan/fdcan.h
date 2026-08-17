/*
 * fdcan.h - FDCAN driver interface (STM32H750, 2x FDCAN controllers)
 */
#ifndef BT_FW_DRV_FDCAN_H
#define BT_FW_DRV_FDCAN_H

#include <stdint.h>

/* Initialize the two FDCAN controllers for 'count' channels starting at
 * 'base_channel' (channel ids must match the PC-side object model). */
void fdcan_init(uint8_t base_channel, uint8_t count);

/* Enable/disable reception filtering (FPGA does the heavy filtering). */
void fdcan_set_filter(uint8_t channel, uint32_t mask, uint32_t value);

/* Transmit one frame; returns 0 on success, negative on busy. */
int fdcan_send_frame(uint8_t channel, uint32_t id, uint8_t extended,
                     uint8_t fd, const uint8_t *data, uint8_t dlc);

/* ISR entry: enqueue received frame into shared ring (non-blocking). */
void fdcan_rx_isr(uint8_t channel);

#endif /* BT_FW_DRV_FDCAN_H */
