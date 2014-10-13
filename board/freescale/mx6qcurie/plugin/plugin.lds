OUTPUT_FORMAT("elf32-littlearm", "elf32-littlearm", "elf32-littlearm")
OUTPUT_ARCH(arm)
ENTRY(plugin_entry)
SEARCH_DIR(.)

FIQ_STACK_SIZE = 0x0100;
IRQ_STACK_SIZE = 0x0100;
ABT_STACK_SIZE = 0x0100;
UND_STACK_SIZE = 0x0100;
SVC_STACK_SIZE = 0x2000;

OCRAM_SIZE_TO_USE = 60K;
STACK_SIZE = FIQ_STACK_SIZE + IRQ_STACK_SIZE + ABT_STACK_SIZE + UND_STACK_SIZE + SVC_STACK_SIZE;

MEMORY
{
	ram : org = 0x0090742c, len = 60K
}

SECTIONS
{
	.init :
	{
		*(.vectors);
		. = ALIGN(4);
		*(.init);
		. = ALIGN(4);
	} > ram

	.data :
	{
		PROVIDE (__data_start = .);
		*(.data)
		. = ALIGN(4);
		edata = .;
		_edata = .;
		PROVIDE (__data_end = .);
	} > ram

	.bss :
	{
		PROVIDE (__bss_start = .);
		*(.bss)
		*(COMMON)
		. = ALIGN(4);
		PROVIDE (__bss_end = .);
	} > ram

	.text :
	{
		*(.rodata);
		. = ALIGN(4);
		*(.rodata*);
		. = ALIGN(4);
		*(.glue_7t);
		. = ALIGN(4);
		*(.glue_7);
		. = ALIGN(4);
		etext = .;
		*(.text);
		. = ALIGN(4);
	} > ram

	. = ALIGN(256);
	_end = .;
	PROVIDE (end = .);

	_end = .;

	. = OCRAM_SIZE_TO_USE - STACK_SIZE;
	PROVIDE (__stack_start = .);

	. = __stack_start;
	PROVIDE (__stack_svc_start = .);
	. += SVC_STACK_SIZE;
	. = ALIGN(4);
	PROVIDE (__stack_svc_end = .);

	PROVIDE (__stack_irq_start = .);
	. += IRQ_STACK_SIZE;
	. = ALIGN(4);
	PROVIDE (__stack_irq_end = .);

	PROVIDE (__stack_fiq_start = .);
	. += FIQ_STACK_SIZE;
	. = ALIGN(4);
	PROVIDE (__stack_fiq_end = .);

	PROVIDE (__stack_abt_start = .);
	. += ABT_STACK_SIZE;
	. = ALIGN(4);
	PROVIDE (__stack_abt_end = .);

	PROVIDE (__stack_und_start = .);
	. += UND_STACK_SIZE;
	. = ALIGN(4);
	PROVIDE (__stack_und_end = .);

	PROVIDE (__stack_end = .);

	STACK_ADR = . ;

	. = ALIGN(4);

}

