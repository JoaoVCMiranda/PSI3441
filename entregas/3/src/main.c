/*
 * PSI3441 — Atividade 3: Pisca LED verde via registradores
 * FRDM-KL25Z
 *
 * Sequência bare-metal (sem abstração do Zephyr GPIO):
 *   (1) Habilita clock da porta B  — SIM_SCGC5, bit 10
 *   (2) Configura PCR do pino 19   — MUX = 001 (GPIO)
 *   (3) Define pino 19 como saída  — GPIOB_PDDR, bit 19
 *   (4) Liga LED   → pino LOW  (active-low) via GPIOB_PCOR
 *   (5) Aguarda 1 s
 *   (6) Apaga LED  → pino HIGH via GPIOB_PSOR
 *   (7) Aguarda 1 s                          repete (4)–(7)
 *
 * Nota: delayMs foi calibrado para o clock default do KL25Z (~21 MHz).
 * Em contexto Zephyr (48 MHz), o período real fica ~860 ms em vez de 2 s.
 * Para corrigir basta mudar o j < 7000 para j < 16000 (proporcional).
 *
 * Build:   pio run -t upload
 */

/* Mapeamento dos registradores pelo endereço físico (KL25Z Reference Manual) */
#define SIM_SCGC5    (*((volatile unsigned int *)0x40048038))
#define PORTB_PCR19  (*((volatile unsigned int *)0x4004A04C))
#define GPIOB_PDOR   (*((volatile unsigned int *)0x400FF040))
#define GPIOB_PSOR   (*((volatile unsigned int *)0x400FF044))  /* set  → pino HIGH */
#define GPIOB_PCOR   (*((volatile unsigned int *)0x400FF048))  /* clear → pino LOW */
#define GPIOB_PDDR   (*((volatile unsigned int *)0x400FF054))

/* Aguarda aproximadamente n milissegundos.
 * j < 7000 calibrado para ~21 MHz; ajuste para j < 16000 se usar Zephyr (48 MHz). */
void delayMs(int n)
{
    int i, j;
    for (i = 0; i < n; i++)
        for (j = 0; j < 7000; j++) {}
}

void main(void)
{
    /* (1) Habilita clock da porta B — bit 10 de SIM_SCGC5 */
    SIM_SCGC5 |= (1 << 10);

    /* (2) Pino 19 da porta B como GPIO — PCR[MUX] = 001 → 0x100 */
    PORTB_PCR19 = 0x100;

    /* (3) Pino 19 como saída — bit 19 do PDDR */
    GPIOB_PDDR |= (1u << 19);

    /* Garante LED apagado antes do loop (pino HIGH = active-low apagado) */
    GPIOB_PSOR = (1u << 19);

    for (;;) {
        /* (4) Liga LED — pino LOW via PCOR (atômico: não afeta outros pinos) */
        GPIOB_PCOR = (1u << 19);
        /* (5) 1 s ligado */
        delayMs(1000);

        /* (6) Apaga LED — pino HIGH via PSOR */
        GPIOB_PSOR = (1u << 19);
        /* (7) 1 s apagado */
        delayMs(1000);
    }
}
