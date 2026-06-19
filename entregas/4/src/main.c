/* PSI3441 — Atividade 4: Aquisição ADC + controle LEDs via Zephyr API */
#include <zephyr.h>
#include <drivers/gpio.h>
#include <drivers/adc.h>

/* ── LEDs via Device Tree ─────────────────────────────────────────────── */
static const struct gpio_dt_spec green_led = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);
static const struct gpio_dt_spec blue_led  = GPIO_DT_SPEC_GET(DT_ALIAS(led1), gpios);

/* ── ADC ──────────────────────────────────────────────────────────────── */
#define ADC_CHANNEL_ID   8       /* ADC0_SE8 = PTB0 (header A0)       */
#define ADC_RESOLUTION   12      /* 12 bits → 0..4095                  */
#define THRESHOLD        2048    /* limiar: ~1,65 V (metade de 3,3 V)  */

static int16_t adc_buf;

static const struct adc_channel_cfg ch_cfg = {
    .gain             = ADC_GAIN_1,
    .reference        = ADC_REF_VDD_1,        /* referência = VDD = 3,3 V */
    .acquisition_time = ADC_ACQ_TIME_DEFAULT,
    .channel_id       = ADC_CHANNEL_ID,
    .differential     = 0,                     /* single-ended             */
};

static struct adc_sequence seq = {
    .channels    = BIT(ADC_CHANNEL_ID),
    .buffer      = &adc_buf,
    .buffer_size = sizeof(adc_buf),
    .resolution  = ADC_RESOLUTION,
};

void main(void) {
    const struct device *adc_dev = DEVICE_DT_GET(DT_NODELABEL(adc0));

    if (!device_is_ready(adc_dev)        ||
        !device_is_ready(green_led.port) ||
        !device_is_ready(blue_led.port)) {
        return;
    }

    gpio_pin_configure_dt(&green_led, GPIO_OUTPUT_INACTIVE);
    gpio_pin_configure_dt(&blue_led,  GPIO_OUTPUT_INACTIVE);

    adc_channel_setup(adc_dev, &ch_cfg);

    for (;;) {
        adc_read(adc_dev, &seq);

        if (adc_buf > THRESHOLD) {
            /* ~3,3 V → LED azul aceso, verde apagado */
            gpio_pin_set_dt(&blue_led,  1);
            gpio_pin_set_dt(&green_led, 0);
        } else {
            /* ~0 V → LED verde aceso, azul apagado */
            gpio_pin_set_dt(&green_led, 1);
            gpio_pin_set_dt(&blue_led,  0);
        }
    }
}