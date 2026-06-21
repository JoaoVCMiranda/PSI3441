/*
 * PSI3441 — Atividade 6: Threads
 * FRDM-KL25Z + Zephyr OS
 *
 * Três fluxos concorrentes criados com K_THREAD_DEFINE:
 *   t_red   — pisca LED vermelho (PTB18) a cada 500 ms  [prio 5]
 *   t_green — pisca LED verde   (PTB19) a cada 200 ms  [prio 6]
 *   main    — imprime uptime a cada 1 s                 [prio 0]
 *
 * O scheduler Zephyr acorda cada thread quando k_msleep() expira.
 * Como os períodos são distintos, os três correm de forma aparentemente
 * paralela num único núcleo Cortex-M0+.
 */

#include <zephyr.h>
#include <drivers/gpio.h>

/* led2 = PTB18 (vermelho, active-low)   led0 = PTB19 (verde, active-low) */
static const struct gpio_dt_spec led_red   = GPIO_DT_SPEC_GET(DT_ALIAS(led2), gpios);
static const struct gpio_dt_spec led_green = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);

#define STACK_SIZE 512

static void fn_red(void *a, void *b, void *c)
{
    gpio_pin_configure_dt(&led_red, GPIO_OUTPUT_INACTIVE);
    for (;;) {
        gpio_pin_toggle_dt(&led_red);
        printk("[red]   %u ms\n", k_uptime_get_32());
        k_msleep(500);
    }
}

static void fn_green(void *a, void *b, void *c)
{
    gpio_pin_configure_dt(&led_green, GPIO_OUTPUT_INACTIVE);
    for (;;) {
        gpio_pin_toggle_dt(&led_green);
        k_msleep(200);
    }
}

/* K_THREAD_DEFINE cria a thread em tempo de compilação (seção .data do ELF).
 * Sintaxe: nome, tamanho da pilha, função, p1, p2, p3, prioridade, opções, delay */
K_THREAD_DEFINE(t_red,   STACK_SIZE, fn_red,   NULL, NULL, NULL, 5, 0, 0);
K_THREAD_DEFINE(t_green, STACK_SIZE, fn_green, NULL, NULL, NULL, 6, 0, 0);

void main(void)
{
    printk("PSI3441 — Atividade 6: Threads\n");
    printk("  t_red   500 ms  prio=5\n");
    printk("  t_green 200 ms  prio=6\n");
    printk("  main   1000 ms  prio=0\n");

    for (;;) {
        printk("[main]  uptime: %u ms\n", k_uptime_get_32());
        k_msleep(1000);
    }
}
