/* PSI3441 — Atividade 5: PWM via TPM0 + medida de distancia com HC-SR04 */

/* SIM */
#define SIM_SOPT2   (*((volatile unsigned int *)0x40048004))
#define SIM_SCGC5   (*((volatile unsigned int *)0x40048038))
#define SIM_SCGC6   (*((volatile unsigned int *)0x4004803C))

/* MCG: habilita Fast IRC (4 MHz) como fonte dos TPMs */
#define MCG_C1      (*((volatile unsigned char *)0x40064000))
#define MCG_C2      (*((volatile unsigned char *)0x40064001))

/* Porta C — PTC1: TPM0_CH0 (ALT4), saida PWM */
#define PORTC_PCR1  (*((volatile unsigned int *)0x4004B004))

/* Porta A — PTA1: TRIG (saida), PTA2: ECHO (entrada) */
#define PORTA_PCR1  (*((volatile unsigned int *)0x40049004))
#define PORTA_PCR2  (*((volatile unsigned int *)0x40049008))
#define GPIOA_PSOR  (*((volatile unsigned int *)0x400FF004))
#define GPIOA_PCOR  (*((volatile unsigned int *)0x400FF008))
#define GPIOA_PDIR  (*((volatile unsigned int *)0x400FF010))
#define GPIOA_PDDR  (*((volatile unsigned int *)0x400FF014))

/* TPM0 — edge-aligned PWM canal 0 em PTC1
   Fonte: MCGIRCCLK 4 MHz; PS=/8 -> 500 kHz
   MOD=499 -> 1 kHz; C0V=249 -> 50% duty */
#define TPM0_SC     (*((volatile unsigned int *)0x40038000))
#define TPM0_CNT    (*((volatile unsigned int *)0x40038004))
#define TPM0_MOD    (*((volatile unsigned int *)0x40038008))
#define TPM0_C0SC   (*((volatile unsigned int *)0x4003800C))
#define TPM0_C0V    (*((volatile unsigned int *)0x40038010))

/* TPM1 — contador livre a 1 MHz para medir echo em us
   Fonte: MCGIRCCLK 4 MHz; PS=/4 -> 1 MHz */
#define TPM1_SC     (*((volatile unsigned int *)0x40039000))
#define TPM1_CNT    (*((volatile unsigned int *)0x40039004))
#define TPM1_MOD    (*((volatile unsigned int *)0x40039008))

#define TRIG        (1u << 1)   /* PTA1 */
#define ECHO        (1u << 2)   /* PTA2 */

/* Espera n microsegundos usando TPM1 a 1 MHz (max 65535 us por chamada) */
static void delayUs(unsigned int n) {
    TPM1_CNT = 0;
    while (TPM1_CNT < n) {}
}

/* Envia pulso de trigger (10 us) e retorna duracao do echo em us.
   Retorna 0 se sem echo em 30 ms (fora de alcance ou sensor ausente). */
static unsigned int measureEcho(void) {
    GPIOA_PSOR = TRIG;
    delayUs(10);
    GPIOA_PCOR = TRIG;

    /* Aguarda rising edge do ECHO */
    TPM1_CNT = 0;
    while (!(GPIOA_PDIR & ECHO))
        if (TPM1_CNT > 30000u) return 0u;

    /* Mede duracao do ECHO */
    TPM1_CNT = 0;
    while (GPIOA_PDIR & ECHO)
        if (TPM1_CNT > 30000u) return 0u;

    return TPM1_CNT;
}

int main(void) {
    /* (1) Habilitar Fast IRC (4 MHz) e seleciona-lo como fonte dos TPMs */
    MCG_C2 |= (1u << 0);            /* IRCS = 1: Fast IRC (4 MHz) */
    MCG_C1 |= (1u << 1);            /* IRCLKEN = 1 */
    SIM_SOPT2 |= (3u << 24);         /* TPMSRC = 11: MCGIRCCLK */

    /* (2) Habilitar clocks das portas A e C e dos modulos TPM0 e TPM1 */
    SIM_SCGC5 |= (1u << 9) | (1u << 11);    /* PORTA (bit 9), PORTC (bit 11) */
    SIM_SCGC6 |= (1u << 24) | (1u << 25);   /* TPM0 (bit 24), TPM1 (bit 25) */

    /* (3) PTC1: ALT4 = TPM0_CH0 (saida PWM) */
    PORTC_PCR1 = (4u << 8);

    /* (4) PTA1 = TRIG (GPIO saida), PTA2 = ECHO (GPIO entrada) */
    PORTA_PCR1 = (1u << 8);
    PORTA_PCR2 = (1u << 8);
    GPIOA_PDDR |= TRIG;
    GPIOA_PCOR  = TRIG;   /* TRIG inicia em LOW */

    /* (5) TPM1: contador livre a 1 MHz (PS=010 = /4, CMOD=01) */
    TPM1_MOD = 0xFFFF;
    TPM1_SC  = (2u) | (1u << 3);

    /* (6) TPM0: edge-aligned PWM canal 0, 1 kHz, 50% duty
       C0SC: MSB=1 (PWM), ELSB=1 (high-true) */
    TPM0_MOD  = 499u;
    TPM0_C0SC = (1u << 5) | (1u << 3);
    TPM0_C0V  = 249u;
    TPM0_SC   = (3u) | (1u << 3);    /* PS=011 = /8, CMOD=01 */

    /* (7) Loop: mede distancia a cada ~100 ms */
    for (;;) {
        unsigned int t_us = measureEcho();
        /* d_cm = t_us / 58  (v_som = 340 m/s, ida e volta)  */
        volatile unsigned int d_cm = t_us / 58u;
        (void)d_cm;   /* leitura via debugger/breakpoint */

        delayUs(60000u);
        delayUs(40000u);
    }

    return 0;
}
