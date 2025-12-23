ENTRY(loader)

/*for true kernel physical kernel should go from 0x00100000 -> 0x00100000 + kernel_size*/
/*for true kernel virtual kernel should go from 0xc0100000 -> 0xc0100000 + kernel_size*/
SECTIONS {
    . = 0x00100000;

    KERNEL_PHYSICAL_START = . - 0x00100000; 

    .text ALIGN(0x1000) :
    {
        *(.text*)
    }

    .rodata ALIGN(0x1000) :
    {
        *(.rodata*)
    }

    .data ALIGN(0x1000) :
    {
        *(.data*)
    }

    .bss ALIGN(0x1000) :
    {
        *(COMMON)
        *(.bss*)
    }

    KERNEL_PHYSICAL_END = .;
}
