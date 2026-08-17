/*
 * main.c - STM32H750 firmware entry point
 *
 * Phase A skeleton: clock init, FDCAN/LIN driver init, USB device init,
 * main polling loop. ISR-driven pieces (FDCAN RX FIFO, USB) only set
 * flags / enqueue into ring buffers.
 */
#include "board.h"
#include "app_config.h"
#include "drivers/fdcan/fdcan.h"
#include "drivers/lin/lin.h"
#include "drivers/usb/usb_device.h"
#include "drivers/dfu/dfu.h"
#include "protocol/usb_protocol_handler.h"

/* Forward declarations of STM32 HAL-level inits (linked in Phase B). */
static void system_clock_init(void);
static void gpio_init(void);

int main(void)
{
    system_clock_init();
    gpio_init();

    fdcan_init(BOARD_FDCAN1_CH_BASE, BOARD_CAN_CHANNELS / 2U);
    lin_init(BOARD_LIN_UART_BASE, BOARD_LIN_CHANNELS);
    usb_device_init();

    /* DFU: jump to bootloader region if requested via magic word. */
    if (dfu_check_request()) {
        dfu_jump_to_bootloader();
    }

    for (;;) {
        /* Poll command queue (filled by USB ISR), dispatch handlers. */
        uint8_t cmd_buf[BT_USB_PACKET_MAX];
        uint16_t cmd_len = 0U;
        if (usb_device_poll_cmd(cmd_buf, sizeof(cmd_buf), &cmd_len) == 0 &&
            cmd_len > 0U) {
            (void)usb_protocol_handle_cmd(cmd_buf, cmd_len);
        }

        /* Drain FPGA RX FIFO -> batch -> USB bulk IN (see Phase A spec). */
        usb_device_poll_tx_stream();
    }
}

static void system_clock_init(void)
{
    /* TODO(Phase A): configure PLL to 480 MHz with HSE = 25 MHz. */
}

static void gpio_init(void)
{
    /* TODO(Phase A): alternate-function mapping per board schematic. */
}
