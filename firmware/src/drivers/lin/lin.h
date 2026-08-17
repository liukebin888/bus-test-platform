/*
 * lin.h - LIN driver interface (2 channels, UART-based master/slave)
 */
#ifndef BT_FW_DRV_LIN_H
#define BT_FW_DRV_LIN_H

#include <stdint.h>

void lin_init(uint8_t base_uart, uint8_t count);

/* Configure LIN schedule (periodic header transmission). */
void lin_set_schedule(uint8_t channel, const void *schedule, uint16_t len);

/* Send a LIN frame (header + response) given ID and data. */
int lin_send_frame(uint8_t channel, uint8_t id, const uint8_t *data,
                   uint8_t len);

void lin_rx_isr(uint8_t channel);

#endif /* BT_FW_DRV_LIN_H */
