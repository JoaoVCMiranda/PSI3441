/*
 * PSI3441 — Atividade 5 (Zephyr híbrido)
 *
 * Radar ultrassônico: HC-SR04 em PTC8/PTC9, LED verde (PTB19) via TPM2_CH1.
 *
 * Abordagem:
 *   - GPIO/console: API nativa do Zephyr (sem tocar SCGC5/PCR/PDDR manualmente)
 *   - Timing do pulso de echo: leitura direta de PDIR — overhead do driver Zephyr
 *     (~alguns µs por chamada) é inaceitável para o loop de medição do HC-SR04
 *   - PWM: pwm_z42 (bare-metal) porque a API Zephyr de PWM não expõe o registro
 *     TPM->MOD para atualização em runtime — esse é o ponto didático central
 */

#include <zephyr.h>
#include <drivers/gpio.h>
#include "pwm_z42.h"
#include <device.h>

/* ── pinos HC-SR04 ───────────────────────────────────────────────────── */
#define PIN_TRIG  8   /* PTC8 — saída (disparo) */
#define PIN_ECHO  9   /* PTC9 — entrada (eco)   */

/* Leitura direta de PDIR no loop crítico de timing.
 * gpio_pin_get() percorre o driver Zephyr a cada chamada; para medir
 * pulsos de <150 µs (distâncias curtas), o overhead vira erro de medição.
 * Zephyr ainda configura MUX e SCGC5 corretamente via gpio_pin_configure(). */
#define ECHO_IS_HIGH()  (GPIOC->PDIR & (1u << PIN_ECHO))

/* ── TPM2 via pwm_z42 ────────────────────────────────────────────────
 * Clock: MCGIRCLK (Fast IRC = 4 MHz, independente do PLL que o Zephyr usa)
 * Prescaler PS=64  →  f_tpm = 4 MHz / 64 = 62 500 Hz  (1 tick = 16 µs)
 *
 * Bare-metal usava PLLFLL~21 MHz / PS=128 ≈ 164 kHz.
 * No Zephyr o PLL está em 48 MHz; manter PS=128 daria 375 kHz e
 * MOD_FAR > 65535 — não cabe em 16 bits. MCGIRCLK resolve o problema.
 *
 *   f_pwm = 62500 / (MOD + 1)
 *   MOD_NEAR = 6250  → ~10 Hz  (objeto perto)
 *   MOD_FAR  = 23150 → ~2.7 Hz (objeto longe)           */
#define MOD_NEAR        6250U
#define MOD_FAR        23150U
#define DUTY_DIV           5U   /* CnV = MOD/5 → 20 % duty */

/* Timeout: 25 ms ≈ 4,3 m (v_som ≈ 343 m/s, ida+volta / 2) */
#define ECHO_TIMEOUT_US  25000U

/* CPU a 48 MHz: cada ciclo = ~20,8 ns → divisor para converter ciclos → µs */
#define CPU_MHZ  48U

/* ── medição do echo ─────────────────────────────────────────────────
 * k_cycle_get_32() retorna ciclos do relógio do sistema (SysTick a 48 MHz).
 * No Cortex-M0+ não há DWT, então Zephyr usa SysTick com extensão por software.
 * Resolução: 1 ciclo ≈ 20 ns — suficiente para HC-SR04 (mínimo ~150 µs).   */
static uint32_t measure_echo_us(const struct device *portc)
{
    uint32_t t, elapsed;

    /* Trigger: 10 µs HIGH — k_busy_wait() faz busy-loop calibrado pela BSP  */
    gpio_pin_set(portc, PIN_TRIG, 1);
    k_busy_wait(10);
    gpio_pin_set(portc, PIN_TRIG, 0);

    /* Aguarda echo subir (início do pulso ultrassônico) */
    t = k_cycle_get_32();
    while (!ECHO_IS_HIGH()) {
        if ((k_cycle_get_32() - t) / CPU_MHZ > ECHO_TIMEOUT_US)
            return ECHO_TIMEOUT_US;   /* sem sensor ou objeto fora do alcance */
    }

    /* Conta ciclos enquanto echo está alto (eco em trânsito) */
    t = k_cycle_get_32();
    while (ECHO_IS_HIGH()) {
        if ((k_cycle_get_32() - t) / CPU_MHZ > ECHO_TIMEOUT_US)
            return ECHO_TIMEOUT_US;
    }
    elapsed = k_cycle_get_32() - t;
    return elapsed / CPU_MHZ;   /* ciclos → µs */
}

void main(void)
{
    /* Zephyr entrega o device já com clock gate habilitado.
     * Bare-metal precisava: SIM->SCGC5 |= SIM_SCGC5_PORTC_MASK             */
   
    //const struct device *portc = device_get_binding("GPIOC");
    const struct device *portc =DEVICE_DT_GET(DT_NODELABEL(gpioc));

// DEBUG
    printk("portc=%p\n", (void *)portc);

    gpio_pin_configure(portc, PIN_TRIG, GPIO_OUTPUT_LOW);
    gpio_pin_configure(portc, PIN_ECHO, GPIO_INPUT);
    printk("gpio ok\n");

    pwm_tpm_Init(TPM2, TPM_MCGIRCLK, MOD_FAR, TPM_CLK, PS_64, EDGE_PWM);
    printk("tpm init ok\n");

    pwm_tpm_Ch_Init(TPM2, 1, TPM_PWM_L, GPIOB, 19);
    pwm_tpm_CnV(TPM2, 1, 0);
    printk("tpm ch ok\n");
    /* gpio_pin_configure() escreve PCR[pin] = MUX(1) + direção em PDDR.
     * Bare-metal: PORTC->PCR[8] = PORT_PCR_MUX(1); GPIOC->PDDR |= TRIG_PIN */
    gpio_pin_configure(portc, PIN_TRIG, GPIO_OUTPUT_LOW);
    gpio_pin_configure(portc, PIN_ECHO, GPIO_INPUT);

    /* ── TPM2 via pwm_z42 — a parte bare-metal que permanece ──────────
     * Zephyr não expõe "mudar MOD em runtime" via pwm_set_cycles() — a API
     * só altera CnV (duty) sem reconfigurar o período. Para o radar, onde
     * a frequência de piscar varia com a distância, precisamos do TPM direto.
     *
     * pwm_tpm_Init: habilita clock do TPM2 (SCGC6), configura SOPT2 e SC   */
    pwm_tpm_Init(TPM2, TPM_MCGIRCLK, MOD_FAR, TPM_CLK, PS_64, EDGE_PWM);

    /* pwm_tpm_Ch_Init: escreve PCR[19] = MUX(3) → PTB19 vira TPM2_CH1.
     * Zephyr não sabe desse pino — mas não conflita: nunca chamamos
     * gpio_pin_configure() para PTB19.                                      */
    pwm_tpm_Ch_Init(TPM2, 1, TPM_PWM_L, GPIOB, 19);
    pwm_tpm_CnV(TPM2, 1, 0);   /* low-true: CnV=0 → sempre HIGH → LED apagado */

    /* printk() usa o console Zephyr (UART0 configurada pelo BSP).
     * Bare-metal precisava de uart_init() com SIM_SOPT2, BDH, BDL, C2...  */
    printk("PSI3441 — HC-SR04 + TPM2 via Zephyr + pwm_z42\n");
    printk("  IRC 4 MHz, PS=64 → MOD_NEAR=%u (~10 Hz)  MOD_FAR=%u (~2.7 Hz)\n",
           MOD_NEAR, MOD_FAR);

    for (;;) {
        uint32_t echo_us = measure_echo_us(portc);
        uint32_t mod = 0;

        if (echo_us >= ECHO_TIMEOUT_US) {
            pwm_tpm_CnV(TPM2, 1, 0);   /* sem objeto → LED apagado */
        } else {
            /* Mapeia distância → período (perto = MOD menor = pisca mais rápido) */
            mod = MOD_NEAR + echo_us * (MOD_FAR - MOD_NEAR) / ECHO_TIMEOUT_US;

            /* Escrita direta — única linha "fora do modelo Zephyr".
             * pwm_z42 não oferece função para alterar MOD em runtime;
             * usar o ponteiro do SDK é a solução bare-metal intencional.    */
            TPM2->MOD = (uint16_t)mod;
            pwm_tpm_CnV(TPM2, 1, (uint16_t)(mod / DUTY_DIV));
        }

        /* Distância: d (cm) = echo_us / 58  (v_som ≈ 343 m/s, ida+volta)  */
        printk("echo=%5u us  dist=%3u cm  mod=%5u\n",
               echo_us,
               echo_us / 58U,
               mod);

        /* HC-SR04 datasheet: >= 60 ms entre triggers para evitar eco falso */
        k_sleep(K_MSEC(60));
    }
}
