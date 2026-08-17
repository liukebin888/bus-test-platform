/*
 * dfu.h - Dual-bank DFU OTA (bootloader region + app banks)
 */
#ifndef BT_FW_DRV_DFU_H
#define BT_FW_DRV_DFU_H

#include <stdint.h>

/* Returns 1 if a DFU jump was requested (magic word in backup SRAM). */
uint8_t dfu_check_request(void);

/* Jump to the bootloader (releases control, resets vector table). */
void dfu_jump_to_bootloader(void);

/* Program one block into the inactive bank; returns 0 on success. */
int dfu_write_block(uint32_t offset, const uint8_t *data, uint32_t len);

/* Validate and activate the new bank (swap), power-loss safe. */
int dfu_activate(void);

#endif /* BT_FW_DRV_DFU_H */
