/*
 * usb_device.h - USB3300 ULPI + STM32H750 OTG HS device interface
 */
#ifndef BT_FW_DRV_USB_DEVICE_H
#define BT_FW_DRV_USB_DEVICE_H

#include <stdint.h>

void usb_device_init(void);

/* Non-blocking: poll a command packet received on EP2 OUT.
 * Returns 0 and fills buf/len when a packet is available. */
int usb_device_poll_cmd(uint8_t *buf, uint16_t cap, uint16_t *len);

/* Stream captured frames as 16-frame bulk packets on EP1 IN. */
void usb_device_poll_tx_stream(void);

/* Send an event packet on EP3 IN (interrupt endpoint). */
int usb_device_send_event(const uint8_t *evt, uint16_t len);

void usb_ep2_out_isr(void);
void usb_ep1_in_isr(void);

#endif /* BT_FW_DRV_USB_DEVICE_H */
