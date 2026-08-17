/*
 * board.h - Board-level configuration (STM32H750VBT6 + USB3300 + FPGA)
 * Pin mapping is a Phase A placeholder; align with the board schematic.
 */
#ifndef BT_FW_BOARD_H
#define BT_FW_BOARD_H

#include <stdint.h>

/* ---- Clocks ---- */
#define BOARD_HSE_HZ             25000000U   /* external HSE (placeholder)   */
#define BOARD_SYSCLK_HZ          480000000U  /* H750 runs at 480 MHz         */
#define BOARD_TCXO_HZ            10000000U   /* TCXO shared with FPGA        */

/* ---- CAN channels (FDCAN1/2 each provide 2 channels) ---- */
#define BOARD_CAN_CHANNELS       4U
#define BOARD_FDCAN1_CH_BASE     0U
#define BOARD_FDCAN2_CH_BASE     2U

/* ---- LIN channels ---- */
#define BOARD_LIN_CHANNELS       2U
#define BOARD_LIN_UART_BASE      0U   /* UART4/5 placeholder                */

/* ---- USB ---- */
#define BOARD_USB_ULPI_ENABLE    1U    /* USB3300 ULPI PHY                  */
#define BOARD_USB_EP_DATA_IN     0x81U
#define BOARD_USB_EP_CMD_OUT     0x02U
#define BOARD_USB_EP_EVT_IN      0x83U
#define BOARD_USB_MPS            1024U  /* bulk max packet size             */

/* ---- DFU ---- */
#define BOARD_DFU_APP_BASE       0x08008000U  /* 32 KB bootloader region    */
#define BOARD_DFU_APP_MAX_SIZE   0x8000U      /* 32 KB per bank             */

/* ---- FPGA interface (register bus) ---- */
#define BOARD_FPGA_REG_BASE      0x60000000U  /* FMC bank (placeholder)     */
#define BOARD_FPGA_TS_LATCH_ADDR 0x00000000U

#endif /* BT_FW_BOARD_H */
