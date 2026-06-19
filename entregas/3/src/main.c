/* PSI3441 — LED verde piscando 2 s via Zephyr GPIO API */
#include <zephyr.h>
#include <drivers/gpio.h>

/* "led0" é o alias definido no DTS do frdm_kl25z → PTB19, active-low */
#define LED_NODE DT_ALIAS(led0) // no arquivo zephyr/prj.conf: CONFIG_GPIO=y

static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED_NODE, gpios);

void main(void) {
    if (!device_is_ready(led.port)) {
        return;
    }

    gpio_pin_configure_dt(&led, GPIO_OUTPUT_ACTIVE);

    for (;;) {
        gpio_pin_toggle_dt(&led);   /* alterna aceso/apagado */
        k_msleep(500);             /* 1 s → período total 2 s */
    }
}