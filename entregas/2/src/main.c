/*
 * PSI3441 — Atividade 2: Controle Interativo de Cor via PWM + Terminal
 * FRDM-KL25Z + Zephyr OS
 *
 * LED RGB (active-low) controlado por TPM via pwm_z42:
 *   Vermelho: PTB18 → TPM2_CH0 (ALT3)
 *   Verde:    PTB19 → TPM2_CH1 (ALT3)
 *   Azul:     PTD1  → TPM0_CH1 (ALT4)
 *
 * Comandos via terminal serial:
 *   r / R  → vermelho  -10% / +10%
 *   g / G  → verde     -10% / +10%
 *   b / B  → azul      -10% / +10%
 *
 * Clock: MCGIRCLK 4 MHz, PS=64 → f_tpm = 62 500 Hz
 *        MOD = 624 → f_pwm = 62 500 / 625 = 100 Hz (sem flickering)
 * Duty:  CnV = nível * MOD / 10  (11 passos: 0 % a 100 %)
 *
 * Build:   pio run -t upload
 * Monitor: pio device monitor --baud 115200
 */

#include <zephyr.h>
#include <drivers/uart.h>
#include "pwm_z42.h"

/* MCGIRCLK (4 MHz) / PS=64 = 62 500 Hz de clock no contador.
 * MOD=624 → período = 625 ticks → f_pwm = 100 Hz.
 * Usando MCGIRCLK em vez de PLLFLL porque o Zephyr eleva o PLL para 48 MHz;
 * com PS=128 o MOD_MAX ultrapassaria 16 bits para frequências baixas.       */
#define TPM_MOD  624U

/* Níveis de brilho: 0 = apagado, 10 = brilho máximo (100 %).
 * CnV = nível * TPM_MOD / 10 → duty proporcional ao nível.
 * LED active-low + TPM_PWM_L: CnV=0 → always-high → LED apagado.           */
static uint8_t lvl_r = 0, lvl_g = 0, lvl_b = 0;

static void apply_color(void)
{
    pwm_tpm_CnV(TPM2, 0, (uint16_t)(lvl_r * TPM_MOD / 10));
    pwm_tpm_CnV(TPM2, 1, (uint16_t)(lvl_g * TPM_MOD / 10));
    pwm_tpm_CnV(TPM0, 1, (uint16_t)(lvl_b * TPM_MOD / 10));
    printk("R=%3u%%  G=%3u%%  B=%3u%%\n",
           lvl_r * 10, lvl_g * 10, lvl_b * 10);
}

void main(void)
{
    /* TPM2: vermelho CH0 (PTB18) e verde CH1 (PTB19).
     * pwm_tpm_Ch_Init escreve PCR[pin] = MUX(3) → função TPM.
     * Zephyr já habilitou o clock de PORTB via CONFIG_GPIO=y.               */
    pwm_tpm_Init(TPM2, TPM_MCGIRCLK, TPM_MOD, TPM_CLK, PS_64, EDGE_PWM);
    pwm_tpm_Ch_Init(TPM2, 0, TPM_PWM_L, GPIOB, 18);
    pwm_tpm_Ch_Init(TPM2, 1, TPM_PWM_L, GPIOB, 19);

    /* TPM0: azul CH1 (PTD1).
     * pwm_tpm_Ch_Init escreve PCR[1] = MUX(4) → TPM0_CH1.                  */
    pwm_tpm_Init(TPM0, TPM_MCGIRCLK, TPM_MOD, TPM_CLK, PS_64, EDGE_PWM);
    pwm_tpm_Ch_Init(TPM0, 1, TPM_PWM_L, GPIOD, 1);

    apply_color();  /* LED apagado no início */

    printk("\nPSI3441 — Atividade 2: Controle de Cor via PWM\n");
    printk("  r/R = vermelho -/+10%%\n");
    printk("  g/G = verde    -/+10%%\n");
    printk("  b/B = azul     -/+10%%\n\n");

    /* uart_poll_in lê diretamente do registrador de dados do UART.
     * Funciona em modo polling (sem CONFIG_UART_INTERRUPT_DRIVEN), que é
     * o padrão quando o console Zephyr só usa uart_poll_out para printk.    */
    const struct device *uart = DEVICE_DT_GET(DT_NODELABEL(uart0));

    for (;;) {
        char c;
        if (uart_poll_in(uart, &c) != 0) {
            k_sleep(K_MSEC(10));  /* cede CPU ao scheduler enquanto aguarda */
            continue;
        }

        switch (c) {
        case 'r': if (lvl_r > 0)  lvl_r--; break;
        case 'R': if (lvl_r < 10) lvl_r++; break;
        case 'g': if (lvl_g > 0)  lvl_g--; break;
        case 'G': if (lvl_g < 10) lvl_g++; break;
        case 'b': if (lvl_b > 0)  lvl_b--; break;
        case 'B': if (lvl_b < 10) lvl_b++; break;
        default: continue;
        }
        apply_color();
    }
}
