/* PSI3441 — Atividade 5: PWM via pwm_z42 (Prof. Rehder) + HC-SR04
 *
 * PWM:   TPM2_CH1 no PTB19 (LED verde, active-low)
 *        PLLFLL ~21 MHz / PS=128 / MOD=999 -> ~164 Hz
 *        Duty cycle proporcional a distancia: mais perto = LED mais brilhante
 *
 * TRIG:  PTA1 (saida GPIO)
 * ECHO:  PTA2 (entrada GPIO)
 */

#include "pwm_z42.h"   /* inclui MKL25Z4.h + aliases + API do TPM */

#define TRIG       (1u << 1)   /* PTA1 */
#define ECHO       (1u << 2)   /* PTA2 */
#define TPM_MODULE 999u
#define MAX_CNT    50000u      /* timeout; proporcional ao alcance maximo */

/* Envia pulso de trigger e retorna contagem proporcional ao echo.
   Retorna MAX_CNT se sem resposta (fora de alcance). */
static unsigned int measureEcho(void) {
    unsigned int cnt = 0;

    GPIOA->PSOR = TRIG;
    for (volatile int i = 0; i < 150; i++) {}  /* ~10 us */
    GPIOA->PCOR = TRIG;

    /* aguarda rising edge do ECHO */
    for (volatile unsigned int t = 0; t < MAX_CNT && !(GPIOA->PDIR & ECHO); t++) {}

    /* conta ate falling edge */
    while ((GPIOA->PDIR & ECHO) && cnt < MAX_CNT) cnt++;

    return cnt;
}

int main(void) {
    /* TRIG: saida GPIO em PTA1 */
    SIM->SCGC5 |= SIM_SCGC5_PORTA_MASK;
    PORTA->PCR[1] = PORT_PCR_MUX(1);
    PORTA->PCR[2] = PORT_PCR_MUX(1);
    GPIOA->PDDR |= TRIG;
    GPIOA->PCOR  = TRIG;

    /* PWM no LED verde (PTB19) via TPM2_CH1
       PLLFLL, MOD=999, PS=128, edge PWM high-true */
    pwm_tpm_Init(TPM2, TPM_PLLFLL, TPM_MODULE, TPM_CLK, PS_128, EDGE_PWM);
    pwm_tpm_Ch_Init(TPM2, 1, TPM_PWM_H, GPIOB, 19);
    pwm_tpm_CnV(TPM2, 1, 0);   /* LED apagado inicialmente */

    for (;;) {
        unsigned int echo = measureEcho();

        /* mais perto (echo menor) -> duty maior -> LED mais brilhante
           LED active-low: high-true PWM, maior CnV = mais tempo LOW = mais brilhante */
        uint16_t duty = (echo >= MAX_CNT)
            ? 0u
            : (uint16_t)(TPM_MODULE - echo * TPM_MODULE / MAX_CNT);

        pwm_tpm_CnV(TPM2, 1, duty);
    }

    return 0;
}
