/*
 * fdcan.c - FDCAN driver skeleton (STM32H750 LL/HAL)
 * Phase A: register-level init is TODO; interface and semantics are fixed.
 */
#include "drivers/fdcan/fdcan.h"
#include "board.h"

void fdcan_init(uint8_t base_channel, uint8_t count)
{
    (void)base_channel;
    (void)count;
    /* TODO(Phase A): FDCAN1/FDCAN2 clock enable, bit-timing (CAN FD 2 Mbit/s
     * data phase), RX FIFO0 DMA, TX event FIFO, interrupt wiring. */
}

void fdcan_set_filter(uint8_t channel, uint32_t mask, uint32_t value)
{
    (void)channel;
    (void)mask;
    (void)value;
    /* TODO(Phase A): FDCAN RxFilter standard/extended. */
}

int fdcan_send_frame(uint8_t channel, uint32_t id, uint8_t extended,
                     uint8_t fd, const uint8_t *data, uint8_t dlc)
{
    (void)channel;
    (void)id;
    (void)extended;
    (void)fd;
    (void)data;
    (void)dlc;
    /* TODO(Phase A): FDCAN_TxBufferElement fill + AddRequest. */
    return -1;
}

void fdcan_rx_isr(uint8_t channel)
{
    (void)channel;
    /* TODO(Phase A): read RxFIFO0 elements, timestamp via FPGA, enqueue. */
}
