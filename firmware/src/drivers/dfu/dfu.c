/*
 * dfu.c - Dual-bank DFU OTA skeleton
 */
#include "drivers/dfu/dfu.h"
#include "board.h"

uint8_t dfu_check_request(void)
{
    /* TODO(Phase A): read magic word from RTC backup register / backup RAM. */
    return 0U;
}

void dfu_jump_to_bootloader(void)
{
    /* TODO(Phase A): reset vector table to BOARD_DFU_APP_BASE - 32K, jump. */
}

int dfu_write_block(uint32_t offset, const uint8_t *data, uint32_t len)
{
    (void)offset;
    (void)data;
    (void)len;
    /* TODO(Phase A): flash erase/write with double-bank CRC verification. */
    return -1;
}

int dfu_activate(void)
{
    /* TODO(Phase A): swap banks, set boot flags, reset. */
    return -1;
}
