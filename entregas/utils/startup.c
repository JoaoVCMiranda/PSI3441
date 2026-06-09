/* Minimal startup for FRDM-KL25Z (Cortex-M0+, MKL25Z128) */

extern unsigned int _estack;
extern unsigned int _sidata, _sdata, _edata;
extern unsigned int _sbss, _ebss;

extern int main(void);

void Reset_Handler(void);
void Default_Handler(void) { for (;;); }

/* Interrupt vector table — must be at 0x00000000 */
__attribute__((section(".isr_vector")))
void (* const vectors[])(void) = {
    (void (*)(void))&_estack,  /* 0: initial stack pointer */
    Reset_Handler,             /* 1: reset */
    Default_Handler,           /* 2: NMI */
    Default_Handler,           /* 3: HardFault */
    0, 0, 0, 0, 0, 0, 0,      /* 4-10: reserved */
    Default_Handler,           /* 11: SVCall */
    0, 0,                      /* 12-13: reserved */
    Default_Handler,           /* 14: PendSV */
    Default_Handler,           /* 15: SysTick */
    /* 16-47: external IRQs — all default */
    Default_Handler, Default_Handler, Default_Handler, Default_Handler,
    Default_Handler, Default_Handler, Default_Handler, Default_Handler,
    Default_Handler, Default_Handler, Default_Handler, Default_Handler,
    Default_Handler, Default_Handler, Default_Handler, Default_Handler,
    Default_Handler, Default_Handler, Default_Handler, Default_Handler,
    Default_Handler, Default_Handler, Default_Handler, Default_Handler,
    Default_Handler, Default_Handler, Default_Handler, Default_Handler,
    Default_Handler, Default_Handler, Default_Handler, Default_Handler,
};

/* Flash configuration field — obrigatorio no KL25Z para nao travar o chip
   Posicao fixa em 0x00000400 no mapa de memoria */
__attribute__((section(".flash_config")))
const unsigned char flash_config[16] = {
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, /* backdoor key */
    0xFF, 0xFF, 0xFF, 0xFF,                         /* FPROT: sem protecao */
    0xFE,                                            /* FSEC: chip nao travado */
    0xFF,                                            /* FOPT: defaults */
    0xFF, 0xFF                                       /* reservado */
};

void Reset_Handler(void) {
    unsigned int *src, *dst;

    /* Copia secao .data da flash para a RAM */
    src = &_sidata;
    dst = &_sdata;
    while (dst < &_edata)
        *dst++ = *src++;

    /* Zera secao .bss */
    dst = &_sbss;
    while (dst < &_ebss)
        *dst++ = 0;

    main();
    for (;;);
}
