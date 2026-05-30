/* PSI3441 — Atividade 3: LED verde piscando com periodo de 2 s usando registradores */

/* SIM_SCGC5: System Clock Gating Control Register 5
   bit 10 = habilita clock da porta B */
#define SIM_SCGC5   (*((volatile unsigned int*)0x40048038))

/* PORTB_PCR19: Pin Control Register do pino PTB19 (LED verde no FRDM-KL25Z)
   base da porta B = 0x4004A000; offset pino 19 = 19*4 = 0x4C */
#define PORTB_PCR19 (*((volatile unsigned int*)0x4004A04C))

/* GPIO Porta B — base 0x400FF040 */
#define GPIOB_PDDR  (*((volatile unsigned int*)0x400FF054)) /* Data Direction Register */
#define GPIOB_PSOR  (*((volatile unsigned int*)0x400FF044)) /* Port Set Output Register   (escreve 1 -> pino HIGH) */
#define GPIOB_PCOR  (*((volatile unsigned int*)0x400FF048)) /* Port Clear Output Register (escreve 1 -> pino LOW)  */

/* Espera n milissegundos.
   Calibrado para clock default do KL25Z (~21 MHz): 7000 iteracoes ~ 1 ms. */
void delayMs(int n) {
    int i, j;
    for (i = 0; i < n; i++)
        for (j = 0; j < 7000; j++) {}
}

int main(void) {
    /* (1) Habilitar clock da porta B */
    SIM_SCGC5 |= (1 << 10);

    /* (2) Configurar Pino 19: MUX = 001 (GPIO), campo bits [10:8] */
    PORTB_PCR19 = 0x00000100;

    /* (3) Setar direcao do Pino 19 como saida */
    GPIOB_PDDR |= (1u << 19);

    /* No FRDM-KL25Z o LED verde e active-low:
         PCOR (clear) -> pino LOW  -> LED aceso
         PSOR (set)   -> pino HIGH -> LED apagado
       Periodo de 2 s = 1 s aceso + 1 s apagado */
    for (;;) {
        /* (4) Habilitar saida: LED aceso */
        GPIOB_PCOR = (1u << 19);

        /* (5) Esperar 1 segundo */
        delayMs(1000);

        /* (6) Desabilitar saida: LED apagado */
        GPIOB_PSOR = (1u << 19);

        /* (7) Esperar 1 segundo */
        delayMs(1000);
    }

    return 0;
}
