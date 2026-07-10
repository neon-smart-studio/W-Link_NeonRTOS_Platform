/******************************************************************************
*
* Copyright (C) 2017 Texas Instruments Incorporated - http://www.ti.com/
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions
* are met:
*
*  Redistributions of source code must retain the above copyright
*  notice, this list of conditions and the following disclaimer.
*
*  Redistributions in binary form must reproduce the above copyright
*  notice, this list of conditions and the following disclaimer in the
*  documentation and/or other materials provided with the
*  distribution.
*
*  Neither the name of Texas Instruments Incorporated nor the names of
*  its contributors may be used to endorse or promote products derived
*  from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
* "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
* LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
* A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
* OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
* SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
* LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
* DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
* THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
* OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
 *****************************************************************************/
ENTRY(Reset_Handler)

MEMORY
{
    FLASH (rx)  : ORIGIN = 0x00000000, LENGTH = 0x00100000
    SRAM  (rwx) : ORIGIN = 0x20000000, LENGTH = 0x00040000
}

__StackTop = ORIGIN(SRAM) + LENGTH(SRAM);

PROVIDE(__stack = __StackTop);
PROVIDE(_estack = __StackTop);

SECTIONS
{
    /*
     * MSP432E411Y startup vector table
     */
    .isr_vector ORIGIN(FLASH) :
    {
        . = ALIGN(4);
        KEEP(*(.isr_vector))
        . = ALIGN(4);
    } > FLASH

    .text :
    {
        . = ALIGN(4);

        *(.text)
        *(.text.*)
        *(.gnu.linkonce.t.*)

        *(.rodata)
        *(.rodata.*)
        *(.gnu.linkonce.r.*)

        KEEP(*(.init))
        KEEP(*(.fini))

        . = ALIGN(4);

        KEEP(*(.ctors))
        KEEP(*(.dtors))

        . = ALIGN(4);

        __preinit_array_start = .;
        KEEP(*(.preinit_array))
        KEEP(*(SORT(.preinit_array.*)))
        __preinit_array_end = .;

        __init_array_start = .;
        KEEP(*(SORT(.init_array.*)))
        KEEP(*(.init_array))
        __init_array_end = .;

        __fini_array_start = .;
        KEEP(*(SORT(.fini_array.*)))
        KEEP(*(.fini_array))
        __fini_array_end = .;

        *(.eh_frame*)

        . = ALIGN(4);
    } > FLASH

    .ARM.extab :
    {
        . = ALIGN(4);
        *(.ARM.extab*)
        *(.gnu.linkonce.armextab.*)
        . = ALIGN(4);
    } > FLASH

    .ARM.exidx :
    {
        . = ALIGN(4);

        __exidx_start = .;

        *(.ARM.exidx*)
        *(.gnu.linkonce.armexidx.*)

        __exidx_end = .;

        . = ALIGN(4);
    } > FLASH

    /*
     * Flash load address for .data
     */
    _etext = .;

    .data : AT(_etext)
    {
        . = ALIGN(4);

        __data_start__ = .;
        _data           = .;
        _sdata          = .;

        /*
         * MSP432E4/TivaWare RAM vector table
         */
        *(vtable)
        *(.vtable)
        *(.vtable.*)

        *(.data)
        *(.data.*)
        *(.gnu.linkonce.d.*)

        . = ALIGN(4);

        __data_end__ = .;
        _edata       = .;
    } > SRAM

    __data_load__ = LOADADDR(.data);
    _ldata        = LOADADDR(.data);
    _sidata       = LOADADDR(.data);

    .bss (NOLOAD) :
    {
        . = ALIGN(4);

        __bss_start__ = .;
        _bss           = .;
        _sbss          = .;

        *(.bss)
        *(.bss.*)
        *(.gnu.linkonce.b.*)
        *(COMMON)

        . = ALIGN(4);

        __bss_end__ = .;
        _ebss       = .;
    } > SRAM

    .noinit (NOLOAD) :
    {
        . = ALIGN(4);

        *(.noinit)
        *(.noinit.*)

        . = ALIGN(4);
    } > SRAM

    end     = .;
    _end    = .;
    __end__ = .;
}