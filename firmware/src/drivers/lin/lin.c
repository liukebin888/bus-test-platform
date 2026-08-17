/*
 * lin.c - LIN driver skeleton (STM32H750 UART-based)
 */
#include "drivers/lin/lin.h"
#include "board.h"

void lin_init(uint8_t base_uart, uint8_t count)
{
    (void)base_uart;
    (void)count;
    /* TODO(Phase A): UART 19200/9600 baud, LIN mode, break detection. */
}

void lin_set_schedule(uint8_t channel, const void *schedule, uint16_t len)
{
    (void)channel;
    (void)schedule;
    (void)len;
    /* TODO(Phase A): schedule table storage + timer-driven dispatch. */
}

int lin_send_frame(uint8_t channel, uint8_t id, const uint8_t *data,
                   uint8_t len)
{
    (void)channel;
    (void)id;
    (void)data;
    (void)len;
    /* TODO(Phase A): send break + sync + PID + data + checksum. */
    return -1;
}

void lin_rx_isr(uint8_t channel)
{
    (void)channel;
    /* TODO(Phase A): frame assembly, checksum verify, enqueue. */
}
