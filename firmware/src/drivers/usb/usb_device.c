/*
 * usb_device.c - USB3300 ULPI device skeleton (STM32H750 OTG HS)
 */
#include "drivers/usb/usb_device.h"
#include "board.h"

void usb_device_init(void)
{
    /* TODO(Phase A): enable OTG_HS clock, ULPI PHY pins (PA0-PA15 alt 10),
     * configure EP1 IN bulk (1024B, multi), EP2 OUT bulk, EP3 IN interrupt,
     * set device descriptor (VID/PID per project), connect. */
}

int usb_device_poll_cmd(uint8_t *buf, uint16_t cap, uint16_t *len)
{
    (void)buf;
    (void)cap;
    (void)len;
    /* TODO(Phase A): drain EP2 OUT RX FIFO into command queue. */
    return -1;
}

void usb_device_poll_tx_stream(void)
{
    /* TODO(Phase A): batch 16 frames from FPGA FIFO into EP1 IN packets,
     * MMAP-style double buffering to avoid interrupt storms. */
}

int usb_device_send_event(const uint8_t *evt, uint16_t len)
{
    (void)evt;
    (void)len;
    /* TODO(Phase A): EP3 interrupt IN transfer. */
    return -1;
}

void usb_ep2_out_isr(void)
{
    /* Enqueue received command packet (non-blocking). */
}

void usb_ep1_in_isr(void)
{
    /* Advance TX double-buffer pointer (non-blocking). */
}
