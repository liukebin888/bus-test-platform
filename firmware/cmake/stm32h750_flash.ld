/* stm32h750_flash.ld - Linker script for STM32H750VBT6 */
/* Flash: 128 KB @ 0x08000000 (single bank; dual-bank OTA splits at 64 KB) */
/* RAM  : 128 KB @ 0x24000000 (AXI SRAM, main data) + ITCM/DTCM excluded */

MEMORY
{
    FLASH (rx)  : ORIGIN = 0x08000000, LENGTH = 128K
    RAM   (rwx) : ORIGIN = 0x24000000, LENGTH = 128K
}

/* Reserve first 32 KB of flash for the bootloader region (double-bank OTA). */
_bootloader_region_size = 32K;

SECTIONS
{
    .isr_vector :
    {
        KEEP(*(.isr_vector))
    } > FLASH

    .text :
    {
        *(.text*)
        *(.rodata*)
        . = ALIGN(4);
        _etext = .;
    } > FLASH

    .data :
    {
        _sdata = .;
        *(.data*)
        . = ALIGN(4);
        _edata = .;
    } > RAM AT > FLASH

    _sidata = LOADADDR(.data);

    .bss (NOLOAD) :
    {
        _sbss = .;
        *(.bss*)
        *(COMMON)
        . = ALIGN(4);
        _ebss = .;
    } > RAM

    . = ALIGN(4);
    _estack = ORIGIN(RAM) + LENGTH(RAM);

    /DISCARD/ :
    {
        *(.ARM.exidx*)
        *(.note*)
        *(.comment)
    }
}
