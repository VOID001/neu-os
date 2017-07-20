SECTIONS {
    ENTRY(startup_32)
    .text 0x0000 : {
        *(.text)
    }

    .rodata . : {
        *(.rodata)
    }

    .data . : {
        *(.data)
    }

    /DISCARD/ : {
        *(.eh_frame)
    }
}
