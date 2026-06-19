# PSI3441 — Relatório da Experiência 4
## Aquisição ADC e Controle de LEDs no FRDM-KL25Z

---

## 1. Objetivo

Implementar um **comparador analógico por software** utilizando o módulo ADC0 do microcontrolador MKL25Z128VLK4 (placa FRDM-KL25Z). Uma tensão analógica aplicada ao pino PTB0 é amostrada em 12 bits e comparada com um limiar de metade da escala (~1,65 V). O resultado determina qual dos dois LEDs da placa — verde (PTB19) ou azul (PTD1) — permanece aceso.

---

## 2. Hardware

### 2.1 Placa FRDM-KL25Z

| Característica | Valor |
|---|---|
| Microcontrolador | MKL25Z128VLK4 |
| Arquitetura | ARM Cortex-M0+ |
| Clock do sistema (Zephyr) | 48 MHz |
| Flash | 128 KB |
| RAM | 16 KB |

### 2.2 Pinagem utilizada

| Pino | Função | Observação |
|---|---|---|
| PTB0 | Entrada analógica — ADC0_SE8 | Header A0 da placa |
| PTB19 | LED verde | Active-low (LOW = aceso) |
| PTD1 | LED azul | Active-low (LOW = aceso) |

### 2.3 Circuito de entrada

Um potenciômetro é conectado entre VDD (3,3 V) e GND, com o cursor ligado ao header A0 (PTB0). Girando o potenciômetro, a tensão no cursor varia continuamente de 0 V a 3,3 V, permitindo observar a comutação dos LEDs ao cruzar o limiar.

```
VDD (3,3 V)
    │
   ┌┴┐
   │ │ potenciômetro
   └┬┘
    ├──────► PTB0 (A0) — entrada ADC
   ┌┴┐
   │ │
   └┬┘
    │
   GND
```

---

## 3. Módulo ADC0 do KL25Z

O MKL25Z128 possui um conversor analógico-digital de aproximação sucessiva (SAR) de até 16 bits. Nesta experiência configura-se:

| Parâmetro | Valor | Justificativa |
|---|---|---|
| Resolução | 12 bits | Faixa 0–4095, suficiente para observar variação contínua |
| Referência de tensão | VDDA = 3,3 V | Coincide com a tensão de alimentação dos sensores |
| Trigger | Software | Conversão iniciada por escrita em SC1A |
| Canal | 8 (ADC0_SE8) | Mapeado para PTB0 com MUX analógico |

### 3.1 Conversão e limiar

Com referência de 3,3 V e resolução de 12 bits, cada LSB corresponde a:

$$\Delta V = \frac{3{,}3\text{ V}}{4095} \approx 0{,}806\text{ mV/LSB}$$

O limiar escolhido é **2048**, que representa exatamente metade da escala:

$$V_{limiar} = 2048 \times 0{,}806\text{ mV} \approx 1{,}65\text{ V}$$

---

## 4. Implementação

### 4.1 Abordagem bare-metal (registradores)

A inicialização do sistema segue a sequência obrigatória do KL25Z:

1. **Clock das portas** — `SIM_SCGC5`: habilitar bits 10 (Port B) e 12 (Port D)
2. **Pino analógico** — `PORTB_PCR0 = 0x00`: MUX = 000 desativa o buffer digital, conectando o pino diretamente ao multiplexador do ADC
3. **Pinos de LED** — `PORTB_PCR19` e `PORTD_PCR1`: MUX = 001 (GPIO)
4. **Direção GPIO** — `GPIOB_PDDR` e `GPIOD_PDDR`: setar bits dos pinos como saída
5. **Clock do ADC** — `SIM_SCGC6` bit 27: habilitar ADC0
6. **Configuração do ADC** — `ADC0_CFG1 = 0x04`: clock = bus clock, modo 12 bits single-ended
7. **Trigger por software** — `ADC0_SC2 = 0x00`: ADTRG = 0

O loop principal opera em **polling**:

```c
for (;;) {
    ADC0_SC1A = ADC_CH8;                    /* inicia conversão no canal 8 */
    while (!(ADC0_SC1A & ADC_COCO)) {}      /* aguarda flag COCO = 1       */
    unsigned int result = ADC0_RA;          /* lê resultado                */

    if (result > THRESHOLD) {
        GPIOD_PCOR = LED_BLUE;              /* acende azul  */
        GPIOB_PSOR = LED_GREEN;             /* apaga verde  */
    } else {
        GPIOB_PCOR = LED_GREEN;             /* acende verde */
        GPIOD_PSOR = LED_BLUE;              /* apaga azul   */
    }
}
```

> **Atenção:** Os LEDs são **active-low** — escrever 1 via `PCOR` (Clear) coloca o pino em nível baixo, acendendo o LED; escrever 1 via `PSOR` (Set) coloca em nível alto, apagando.

### 4.2 Abordagem via API Zephyr

A mesma lógica reescrita com as APIs de alto nível do Zephyr 2.7.1:

```c
#include <zephyr.h>
#include <drivers/gpio.h>
#include <drivers/adc.h>

static const struct gpio_dt_spec green_led = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);
static const struct gpio_dt_spec blue_led  = GPIO_DT_SPEC_GET(DT_ALIAS(led1), gpios);

#define ADC_CHANNEL_ID   8
#define ADC_RESOLUTION   12
#define THRESHOLD        2048

static int16_t adc_buf;

static const struct adc_channel_cfg ch_cfg = {
    .gain             = ADC_GAIN_1,
    .reference        = ADC_REF_VDD_1,
    .acquisition_time = ADC_ACQ_TIME_DEFAULT,
    .channel_id       = ADC_CHANNEL_ID,
    .differential     = 0,
};

static struct adc_sequence seq = {
    .channels    = BIT(ADC_CHANNEL_ID),
    .buffer      = &adc_buf,
    .buffer_size = sizeof(adc_buf),
    .resolution  = ADC_RESOLUTION,
};

void main(void) {
    const struct device *adc_dev = DEVICE_DT_GET(DT_NODELABEL(adc0));

    gpio_pin_configure_dt(&green_led, GPIO_OUTPUT_INACTIVE);
    gpio_pin_configure_dt(&blue_led,  GPIO_OUTPUT_INACTIVE);
    adc_channel_setup(adc_dev, &ch_cfg);

    for (;;) {
        adc_read(adc_dev, &seq);

        if (adc_buf > THRESHOLD) {
            gpio_pin_set_dt(&blue_led,  1);
            gpio_pin_set_dt(&green_led, 0);
        } else {
            gpio_pin_set_dt(&green_led, 1);
            gpio_pin_set_dt(&blue_led,  0);
        }
    }
}
```

`prj.conf`:
```ini
CONFIG_GPIO=y
CONFIG_ADC=y
```

#### Correspondência entre as abordagens

| Bare-metal | Zephyr API | Descrição |
|---|---|---|
| `SIM_SCGC5/6` | automático pelo driver | habilitação de clock |
| `PORTB_PCR0 = 0x00` | `ch_cfg.differential = 0` | modo analógico single-ended |
| `ADC0_CFG1 = 0x04` | `ch_cfg + seq.resolution` | resolução e clock do ADC |
| `ADC0_SC1A = CH8; while(!COCO)` | `adc_read()` | início e espera da conversão |
| `ADC0_RA` | `adc_buf` | leitura do resultado |
| `GPIOB_PCOR/PSOR` | `gpio_pin_set_dt()` | controle dos LEDs |

A API Zephyr abstrai a polaridade dos LEDs via Device Tree (`GPIO_ACTIVE_LOW` já declarado no DTS da placa), eliminando a necessidade de raciocinar sobre PSOR/PCOR para cada LED.

---

## 5. Resultados Esperados

| Posição do potenciômetro | Tensão em A0 | Valor ADC | LED aceso |
|---|---|---|---|
| Girado para VDD | ~3,3 V | ~4095 | 🔵 Azul |
| Centro | ~1,65 V | ~2048 | transição |
| Girado para GND | ~0 V | ~0 | 🟢 Verde |

A comutação ocorre instantaneamente ao cruzar 1,65 V, sem histerese — comportamento esperado de um comparador ideal implementado em software.

---

## 6. Conclusão

A experiência demonstrou o fluxo completo de aquisição analógica no KL25Z: habilitação de clocks, configuração do multiplexador de pinos, parametrização do módulo ADC e leitura por polling do flag de conversão completa (COCO). A comparação com o limiar de metade da escala e o acionamento dos LEDs consolidam o uso de periféricos GPIO e ADC em conjunto.

A reescrita via API Zephyr evidencia as vantagens de uma camada de abstração de hardware: o código torna-se independente de endereços de registrador, a polaridade dos LEDs é tratada pelo Device Tree, e `adc_read()` encapsula o protocolo de iniciar e aguardar a conversão.