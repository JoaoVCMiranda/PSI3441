/* PSI3441 — Atividade 1: Pisca LED
 * FRDM-KL25Z + Zephyr OS
 *
 * Pisca o LED vermelho (PTB18, active-low) a cada 500 ms via API GPIO Zephyr.
 * O alias "led2" é definido no DTS do frdm_kl25z e aponta para PTB18.
 *
 * Build:   pio run -t upload
 * Monitor: pio device monitor --baud 115200
 */

#include <zephyr.h>
#include <drivers/gpio.h>

#define LED_NODE DT_ALIAS(led2)  /* led2 = PTB18 (vermelho), active-low */

static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED_NODE, gpios);

void main(void)
{
    gpio_pin_configure_dt(&led, GPIO_OUTPUT_INACTIVE);  /* começa apagado */

    printk("PSI3441 — Atividade 1: Pisca LED\n");

    for (;;) {
        gpio_pin_toggle_dt(&led);
        k_msleep(500);   /* alterna a cada 500 ms → período total 1 s */
    }
}
