/*
 * PSI3441 — Entrega N (template)
 * FRDM-KL25Z + Zephyr OS
 *
 * Ponto de partida: pisca o LED vermelho (PTB18) via GPIO Zephyr
 * e imprime no console serial (cat /dev/ttyACM0).
 *
 * Build:  pio run -t upload        (PlatformIO)
 *         west build -b frdm_kl25z (west direto, app dir = zephyr/)
 * Monitor: pio device monitor --baud 115200
 */

#include <zephyr.h>
#include <drivers/gpio.h>

/* LED vermelho = PTB18, active-low */
#define LED_PORT   "GPIOB"
#define LED_PIN    18

void main(void)
{
    const struct device *led = device_get_binding(LED_PORT);

    /* active-low: GPIO_OUTPUT_HIGH deixa o LED apagado inicialmente */
    gpio_pin_configure(led, LED_PIN, GPIO_OUTPUT_HIGH);

    printk("PSI3441 — template inicializado\n");

    for (;;) {
        gpio_pin_toggle(led, LED_PIN);
        printk("tick\n");
        k_sleep(K_MSEC(500));
    }
}
