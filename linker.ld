ENTRY(start)

SECTIONS {
    . = 1M;

    .boot :
    {
        /* ensure that the multiboot header is at the beginning */
        *(.multiboot_header)
    }

    .text :
    {
        *(.text)
    }
    .idt BLOCK(0x1000) : ALIGN(0x1000)
    {
    IDT = .;
    . = . + 0x1000;
    }
}

