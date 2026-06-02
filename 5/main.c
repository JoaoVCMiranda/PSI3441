/* PSI3441 — Atividade 5: PWM via pwm_z42 + HC-SR04 + UART debug
 *
 * Radar:  TPM2_CH1 / PTB19 (LED verde, active-low, low-true PWM)
 *         mais perto = pisca mais rapido; sem objeto = LED apagado
 *
 * HC-SR04: TRIG = PTC8 (J1-9)   ECHO = PTC9 (J1-10)
 *
 * Debug:  UART0 115200 8N1 via DAPLINK USB-serial
 *           cat /dev/ttyACM0        (Linux)
 *           screen /dev/ttyACM0 115200
 */

#include "pwm_z42.h"

/* HC-SR04 em PORTC */
#define TRIG_PIN    (1u << 8)   /* PTC8 */
#define ECHO_PIN    (1u << 9)   /* PTC9 */

/* Frequencias do radar — TPM clock = PLLFLL~21 MHz / PS=128 = ~164 kHz
 *   MOD=16000 -> f = 164000/16001 ~ 10 Hz  (perto)
 *   MOD=60000 -> f = 164000/60001 ~  2.7 Hz (longe)  */
#define BLINK_NEAR  16000u
#define BLINK_FAR   60000u
#define DUTY_DIV    5u          /* CnV = MOD/5 = 20 % duty */

#define MAX_CNT     50000u

/* ── UART0 debug (PTA1=RX, PTA2=TX) ─────────────────────────────── */

static void uart_init(void) {
    SIM->SCGC4 |= SIM_SCGC4_UART0_MASK;
    SIM->SOPT2 |= SIM_SOPT2_UART0SRC(1);   /* MCGFLLCLK ~21 MHz */
    PORTA->PCR[1] = PORT_PCR_MUX(2);        /* UART0_RX */
    PORTA->PCR[2] = PORT_PCR_MUX(2);        /* UART0_TX */
    UART0_BASE_PTR->BDH = 0;
    UART0_BASE_PTR->BDL = 11;               /* SBR=11 -> ~119200 baud @ 21 MHz */
    UART0_BASE_PTR->C1  = 0;
    UART0_BASE_PTR->C2  = UART0_C2_TE_MASK;
}

static void uart_putc(char c) {
    while (!(UART0_BASE_PTR->S1 & UART0_S1_TDRE_MASK)) {}
    UART0_BASE_PTR->D = (unsigned char)c;
}

static void uart_putu(unsigned int n) {
    if (n >= 10u) uart_putu(n / 10u);
    uart_putc('0' + (char)(n % 10u));
}

/* ── HC-SR04 ──────────────────────────────────────────────────────── */

static unsigned int measureEcho(void) {
    unsigned int cnt = 0, t;

    GPIOC->PSOR = TRIG_PIN;
    for (volatile int i = 0; i < 150; i++) {}   /* ~10 us */
    GPIOC->PCOR = TRIG_PIN;

    /* aguarda rising edge; retorna MAX_CNT se nao houver eco */
    for (t = 0; t < MAX_CNT && !(GPIOC->PDIR & ECHO_PIN); t++) {}
    if (t >= MAX_CNT) return MAX_CNT;

    /* conta ate falling edge */
    while ((GPIOC->PDIR & ECHO_PIN) && cnt < MAX_CNT) cnt++;
    return cnt ? cnt : 1u;
}

/* ── main ─────────────────────────────────────────────────────────── */

int main(void) {
    /* clocks das portas */
    SIM->SCGC5 |= SIM_SCGC5_PORTA_MASK | SIM_SCGC5_PORTC_MASK;

    uart_init();

    /* PTC8 = TRIG (saida), PTC9 = ECHO (entrada, pull-down) */
    PORTC->PCR[8] = PORT_PCR_MUX(1);
    PORTC->PCR[9] = PORT_PCR_MUX(1) | PORT_PCR_PE_MASK;   /* PE=1, PS=0 = pull-down */
    GPIOC->PDDR |= TRIG_PIN;
    GPIOC->PCOR  = TRIG_PIN;

    /* PWM: TPM2_CH1 / PTB19, low-true (ELSA=1), LED off = CnV=0 */
    pwm_tpm_Init(TPM2, TPM_PLLFLL, BLINK_FAR, TPM_CLK, PS_128, EDGE_PWM);
    pwm_tpm_Ch_Init(TPM2, 1, TPM_PWM_L, GPIOB, 19);
    pwm_tpm_CnV(TPM2, 1, 0);   /* CnV=0 -> sempre HIGH -> LED apagado */

    for (;;) {
        unsigned int echo = measureEcho();
        unsigned int mod  = TPM2_BASE_PTR->MOD;
        unsigned int cnv;

        if (echo >= MAX_CNT) {
            /* sem objeto: LED apagado */
            cnv = 0;
        } else {
            /* radar: echo menor -> MOD menor -> frequencia maior -> pisca mais rapido */
            mod = BLINK_NEAR + echo * (BLINK_FAR - BLINK_NEAR) / MAX_CNT;
            cnv = mod / DUTY_DIV;
            TPM2_BASE_PTR->MOD = (uint16_t)mod;
        }
        pwm_tpm_CnV(TPM2, 1, (uint16_t)cnv);

        /* debug: "echo mod\n" */
        uart_putu(echo);
        uart_putc(' ');
        uart_putu(mod);
        uart_putc('\n');

        /* >= 60 ms entre triggers (datasheet HC-SR04) */
        for (volatile unsigned int i = 0; i < 400000u; i++) {}
    }

    return 0;
}
