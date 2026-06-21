/*
 * PSI3441 — Atividade 7: Shared Resources
 * FRDM-KL25Z + Zephyr OS
 *
 * Recurso compartilhado: contador shared_count protegido por k_mutex.
 *
 *   t_producer — incrementa shared_count a cada 100 ms, pisca LED vermelho
 *   t_consumer — lê, imprime e zera shared_count a cada 500 ms, pisca LED verde
 *
 * Sem mutex, o consumer poderia ler um valor inconsistente: o compilador
 * pode manter shared_count em registrador entre operações, e uma preempção
 * entre leitura e zeragem perderia incrementos.
 *
 * No Cortex-M0+ (single-core), race conditions clássicas são menos visíveis,
 * mas o mutex também garante visibility (barreiras de memória) e é necessário
 * em qualquer arquitetura portável.
 */

#include <zephyr.h>
#include <drivers/gpio.h>

static const struct gpio_dt_spec led_red   = GPIO_DT_SPEC_GET(DT_ALIAS(led2), gpios);
static const struct gpio_dt_spec led_green = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);

static uint32_t shared_count;
K_MUTEX_DEFINE(count_mutex);

#define STACK_SIZE 512

static void fn_producer(void *a, void *b, void *c)
{
    gpio_pin_configure_dt(&led_red, GPIO_OUTPUT_INACTIVE);
    for (;;) {
        /* seção crítica: incremento atômico protegido pelo mutex */
        k_mutex_lock(&count_mutex, K_FOREVER);
        shared_count++;
        k_mutex_unlock(&count_mutex);

        gpio_pin_toggle_dt(&led_red);
        k_msleep(100);
    }
}

static void fn_consumer(void *a, void *b, void *c)
{
    gpio_pin_configure_dt(&led_green, GPIO_OUTPUT_INACTIVE);
    uint32_t snap;
    for (;;) {
        k_msleep(500);

        /* lê e zera atomicamente — sem mutex, read-modify poderia ser preemptado */
        k_mutex_lock(&count_mutex, K_FOREVER);
        snap = shared_count;
        shared_count = 0;
        k_mutex_unlock(&count_mutex);

        gpio_pin_toggle_dt(&led_green);
        /* esperado: ~5 incrementos a cada 500 ms (100 ms × 5) */
        printk("eventos/500ms: %u\n", snap);
    }
}

K_THREAD_DEFINE(t_prod, STACK_SIZE, fn_producer, NULL, NULL, NULL, 5, 0, 0);
K_THREAD_DEFINE(t_cons, STACK_SIZE, fn_consumer, NULL, NULL, NULL, 6, 0, 0);

void main(void)
{
    printk("PSI3441 — Atividade 7: Shared Resources\n");
    printk("  producer: +1 a cada 100 ms\n");
    printk("  consumer: snapshot+reset a cada 500 ms\n");
    k_sleep(K_FOREVER);
}
