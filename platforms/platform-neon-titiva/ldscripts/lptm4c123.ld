ENTRY(ResetISR)

MEMORY
{
    FLASH (rx)  : ORIGIN = 0x00000000, LENGTH = 0x00040000  /* 256KB */
    SRAM  (rwx) : ORIGIN = 0x20000000, LENGTH = 0x00008000  /* 32KB */
}

_estack = ORIGIN(SRAM) + LENGTH(SRAM);

SECTIONS
{
    .isr_vector :
    {
        KEEP(*(.isr_vector))
        KEEP(*(.intvecs))      /* TI/TivaWare 常用 */
    } > FLASH

    .text :
    {
        _text = .;

        *(.text .text.*)
        *(.gnu.linkonce.t.*)

        *(.rodata .rodata.*)
        *(.gnu.linkonce.r.*)

        KEEP(*(.init))
        KEEP(*(.fini))

        . = ALIGN(4);
        KEEP(*(.ctors))
        KEEP(*(.init_array*))
        KEEP(*(.dtors))
        KEEP(*(.fini_array*))

        *(.eh_frame*)
    } > FLASH

    .ARM.exidx :
    {
        __exidx_start = .;
        *(.ARM.exidx* .gnu.linkonce.armexidx.*)
        __exidx_end = .;
    } > FLASH

    . = ALIGN(4);
    _etext = .;

    .data : AT(_etext)
    {
        . = ALIGN(4);
        _data = .;
        _ldata = LOADADDR(.data);

        *(vtable)
        *(.data .data.*)
        *(.gnu.linkonce.d.*)

        . = ALIGN(4);
        _edata = .;
    } > SRAM

    .bss (NOLOAD) :
    {
        . = ALIGN(4);
        _bss = .;

        *(.bss .bss.*)
        *(.gnu.linkonce.b.*)
        *(COMMON)

        . = ALIGN(4);
        _ebss = .;
    } > SRAM

    .heap (NOLOAD) :
    {
        . = ALIGN(8);
        _heap_start = .;
        *(.heap*)
        . = ALIGN(8);
        _heap_end = .;
    } > SRAM

    _end = .;
}