/* Linker script to configure memory regions. */
APP_START = DEFINED(APP_START_LOC) ? APP_START_LOC : 0 ;
APP_SIZE = DEFINED(APP_START_LOC) ? 0x80000 - APP_START : 0x80000 ;
FREE_RAM_REQ = DEFINED(MIN_FREE_RAM) ? MIN_FREE_RAM : 0;

MEMORY
{
  FLASH (rx) : ORIGIN = APP_START, LENGTH = APP_SIZE   /* 512k */
  RAM (rwx)  : ORIGIN = 0x20000000, LENGTH = 0x10000   /*  64k */
  RAM_EXTERN (rwx)  : ORIGIN = 0x60000000, LENGTH = 0x100000   /*  1024k */
}

/* Library configurations */
GROUP(libgcc.a libc.a libm.a libnosys.a)

/* Linker script to place sections and symbol values. Should be used together
 * with other linker script that defines memory regions FLASH and RAM.
 * It references following symbols, which must be defined in code:
 *   Reset_Handler : Entry of reset handler
 *
 * It defines following symbols, which code can use without definition:
 *   __exidx_start
 *   __exidx_end
 *   __copy_table_start__
 *   __copy_table_end__
 *   __zero_table_start__
 *   __zero_table_end__
 *   __etext
 *   __data_start__
 *   __preinit_array_start
 *   __preinit_array_end
 *   __init_array_start
 *   __init_array_end
 *   __fini_array_start
 *   __fini_array_end
 *   __data_end__
 *   __bss_start__
 *   __bss_end__
 *   __end__
 *   end
 *   __HeapLimit
 *   __StackLimit
 *   __StackTop
 *   __stack
 *   __Vectors_End
 *   __Vectors_Size
 */
ENTRY(Reset_Handler)

SECTIONS
{
	.text :
	{
		KEEP(*(.vectors))
		__Vectors_End = .;
		__Vectors_Size = __Vectors_End - __Vectors;
		__end__ = .;

		*(.text*)

		KEEP(*(.init))
		KEEP(*(.fini))

		/* .ctors */
		*crtbegin.o(.ctors)
		*crtbegin?.o(.ctors)
		*(EXCLUDE_FILE(*crtend?.o *crtend.o) .ctors)
		*(SORT(.ctors.*))
		*(.ctors)

		/* .dtors */
 		*crtbegin.o(.dtors)
 		*crtbegin?.o(.dtors)
 		*(EXCLUDE_FILE(*crtend?.o *crtend.o) .dtors)
 		*(SORT(.dtors.*))
 		*(.dtors)

		*(.rodata*)

		KEEP(*(.eh_frame*))
		
		__data_section_table = .;
		LONG(LOADADDR(.data));
		LONG(    ADDR(.data));
		LONG(  SIZEOF(.data));
		__data_section_table_end = .;

		__data_extern_section_table = .;
		LONG(LOADADDR(.data.extern));
		LONG(    ADDR(.data.extern));
		LONG(  SIZEOF(.data.extern));
		__data_extern_section_table_end = .;

		__bss_section_table = .;
		LONG(    ADDR(.bss));
		LONG(  SIZEOF(.bss));
		__bss_section_table_end = .;

		__bss_extern_section_table = .;
		LONG(    ADDR(.bss.extern));
		LONG(  SIZEOF(.bss.extern));
		__bss_extern_section_table_end = .;

		
	} > FLASH

	.ARM.extab :
	{
		*(.ARM.extab* .gnu.linkonce.armextab.*)
	} > FLASH

	__exidx_start = .;
	.ARM.exidx :
	{
		*(.ARM.exidx* .gnu.linkonce.armexidx.*)
	} > FLASH
	__exidx_end = .;

	/* To copy multiple ROM to RAM sections,
	 * uncomment .copy.table section and,
	 * define __STARTUP_COPY_MULTIPLE in startup_ARMCMx.S */
	/*
	.copy.table :
	{
		. = ALIGN(4);
		__copy_table_start__ = .;
		LONG (__etext)
		LONG (__data_start__)
		LONG (__data_end__ - __data_start__)
		LONG (__etext2)
		LONG (__data2_start__)
		LONG (__data2_end__ - __data2_start__)
		__copy_table_end__ = .;
	} > FLASH
	*/

	/* To clear multiple BSS sections,
	 * uncomment .zero.table section and,
	 * define __STARTUP_CLEAR_BSS_MULTIPLE in startup_ARMCMx.S */
	/*
	.zero.table :
	{
		. = ALIGN(4);
		__zero_table_start__ = .;
		LONG (__bss_start__)
		LONG (__bss_end__ - __bss_start__)
		LONG (__bss2_start__)
		LONG (__bss2_end__ - __bss2_start__)
		__zero_table_end__ = .;
	} > FLASH
	*/

	__etext = ALIGN (4);

	.data.extern : AT (__etext)
    {
		/**
		* Place large .data* sections into external SRAM if internal SRAM is insufficient.
		* Such memory arrangement requires initializing .data.extern section in startup file. Check startup fiile in cmsis-core-* for support or not.
		*/

		. = ALIGN(4) ;
		__data_extern_start__ = .;
		*(.data.extern*)
		*(.data.$RAM2*)
		*\libtest.a:(.data*)		/*add libcli_service data section to external ram*/
		. = ALIGN(4) ;
		__data_extern_end__ = .;
    } > RAM_EXTERN
    
   	__etext2 = __etext + ALIGN (SIZEOF(.data.extern), 4);

	.data : AT (__etext2)
	{
		__data_start__ = .;
		*(vtable)
		*(.data*)
		*\libc.a:(.data*)
		. = ALIGN(4);
		/* preinit data */
		PROVIDE_HIDDEN (__preinit_array_start = .);
		KEEP(*(.preinit_array))
		PROVIDE_HIDDEN (__preinit_array_end = .);

		. = ALIGN(4);
		/* init data */
		PROVIDE_HIDDEN (__init_array_start = .);
		KEEP(*(SORT(.init_array.*)))
		KEEP(*(.init_array))
		PROVIDE_HIDDEN (__init_array_end = .);


		. = ALIGN(4);
		/* finit data */
		PROVIDE_HIDDEN (__fini_array_start = .);
		KEEP(*(SORT(.fini_array.*)))
		KEEP(*(.fini_array))
		PROVIDE_HIDDEN (__fini_array_end = .);

		KEEP(*(.jcr*))
		. = ALIGN(4);
		/* All data end */
		__data_end__ = .;

	} > RAM 

	.bss.extern (NOLOAD):
    {
		/**
		* Place large .bss* sections into external SRAM if internal SRAM is insufficient.
		* Such memory arrangement requires initializing .bss.extern section to zero in startup file. Check startup fiile in cmsis-core-* for support or not.
		*/

		. = ALIGN(4) ;
		__bss_extern_start__ = .;
		*(.bss.extern*)
		*(.bss.$RAM2*)
		*\libtest.a:(.bss*)			/*add libcli_service data section to external ram*/
		*\libtest.a:(COMMON)		/*add libcli_service data section to external ram*/
		. = ALIGN(4) ;
		__bss_extern_end__ = .;
    } > RAM_EXTERN

	.bss :
	{
		. = ALIGN(4);
		__bss_start__ = .;
		*(.bss*)
		*\libc.a:(.bss*)
		*\libc.a:(COMMON)
		*(COMMON)
		. = ALIGN(4);
		__bss_end__ = .;
	} > RAM

	.heap (COPY):
	{
		__HeapBase = .;
		__end__ = .;
		end = __end__;
		KEEP(*(.heap*))
		__HeapLimit = .;
	} > RAM

	/* .stack_dummy section doesn't contains any symbols. It is only
	 * used for linker to calculate size of stack sections, and assign
	 * values to stack symbols later */
	.stack_dummy (COPY):
	{
		KEEP(*(.stack*))
	} > RAM

	/* Set stack top to end of RAM, and stack limit move down by
	 * size of stack_dummy section */
	__StackTop = ORIGIN(RAM) + LENGTH(RAM);
	__StackLimit = __StackTop - SIZEOF(.stack_dummy);
	PROVIDE(__stack = __StackTop);

	/* Check if data + heap + stack exceeds RAM limit */
	ASSERT(__StackLimit >= __HeapLimit, "region RAM overflowed with stack")
}
