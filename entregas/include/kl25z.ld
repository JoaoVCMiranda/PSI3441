/* Linker script minimo para FRDM-KL25Z (MKL25Z128)
   Flash: 128 KB @ 0x00000000
   RAM:    16 KB @ 0x1FFFF000 (4 KB SRAM_L + 12 KB SRAM_U, contiguos) */

MEMORY {
    FLASH (rx)  : ORIGIN = 0x00000000, LENGTH = 128K
    RAM   (rwx) : ORIGIN = 0x1FFFF000, LENGTH = 16K
}

ENTRY(Reset_Handler)

_estack = ORIGIN(RAM) + LENGTH(RAM);  /* topo da pilha = 0x20003000 */

SECTIONS {
    /* Tabela de vetores — inicio do flash */
    .isr_vector : {
        KEEP(*(.isr_vector))
    } > FLASH

    /* Campo de configuracao do flash — posicao FIXA em 0x400 */
    .flash_config 0x400 : {
        KEEP(*(.flash_config))
    } > FLASH

    /* Codigo e dados const */
    .text : {
        *(.text*)
        *(.rodata*)
        . = ALIGN(4);
        _sidata = .;  /* LMA do .data: onde o conteudo inicial fica na flash */
    } > FLASH

    /* Dados inicializados: ficam na flash (LMA = _sidata), rodam na RAM (VMA) */
    .data : AT(_sidata) {
        _sdata = .;
        *(.data*)
        . = ALIGN(4);
        _edata = .;
    } > RAM

    /* Dados nao inicializados */
    .bss : {
        _sbss = .;
        *(.bss*)
        *(COMMON)
        . = ALIGN(4);
        _ebss = .;
    } > RAM
}
