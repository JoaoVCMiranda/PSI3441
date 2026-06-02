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
    /* pull-down em PTA2: evita leitura HIGH espuria quando sensor nao esta ativo */
    PORTA->PCR[2] = PORT_PCR_MUX(1) | PORT_PCR_PE_MASK;  /* PE=1, PS=0 = pull-down */
    GPIOA->PDDR |= TRIG;
    GPIOA->PCOR  = TRIG;

    /* PWM no LED verde (PTB19) via TPM2_CH1
       PLLFLL, MOD=999, PS=128, edge PWM high-true */
    pwm_tpm_Init(TPM2, TPM_PLLFLL, TPM_MODULE, TPM_CLK, PS_128, EDGE_PWM);
    pwm_tpm_Ch_Init(TPM2, 1, TPM_PWM_H, GPIOB, 19);
    pwm_tpm_CnV(TPM2, 1, 0);   /* LED apagado inicialmente */

    for (;;) {
        unsigned int echo = measureEcho();

        /* echo==0: ECHO nunca subiu (sem sensor/fora de alcance) -> LED apagado
           echo>=MAX_CNT: timeout (objeto muito longe)           -> LED apagado
           caso normal: mais perto (echo menor) -> duty maior -> LED mais brilhante */
        uint16_t duty = (echo == 0 || echo >= MAX_CNT)
            ? 0u
            : (uint16_t)(TPM_MODULE - echo * TPM_MODULE / MAX_CNT);

        pwm_tpm_CnV(TPM2, 1, duty);

        /* HC-SR04 requer >= 60 ms entre triggers para nao travar o sensor */
        for (volatile unsigned int i = 0; i < 400000u; i++) {}
    }

    return 0;
}
