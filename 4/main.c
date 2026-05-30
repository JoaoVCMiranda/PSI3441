/* PSI3441 — Atividade 4: Aquisicao ADC + controle de LEDs azul/verde */

/* SIM — System Integration Module */
#define SIM_SCGC5   (*((volatile unsigned int*)0x40048038))
#define SIM_SCGC6   (*((volatile unsigned int*)0x4004803C))

/* Pin Control Registers
   PTB0  = ADC0_SE8 (entrada analogica — header A0)
   PTB19 = LED verde (active-low)
   PTD1  = LED azul  (active-low) */
#define PORTB_PCR0  (*((volatile unsigned int*)0x4004A000))
#define PORTB_PCR19 (*((volatile unsigned int*)0x4004A04C))
#define PORTD_PCR1  (*((volatile unsigned int*)0x4004C004))

/* GPIO Porta B (base 0x400FF040) */
#define GPIOB_PSOR  (*((volatile unsigned int*)0x400FF044))
#define GPIOB_PCOR  (*((volatile unsigned int*)0x400FF048))
#define GPIOB_PDDR  (*((volatile unsigned int*)0x400FF054))

/* GPIO Porta D (base 0x400FF0C0) */
#define GPIOD_PSOR  (*((volatile unsigned int*)0x400FF0C4))
#define GPIOD_PCOR  (*((volatile unsigned int*)0x400FF0C8))
#define GPIOD_PDDR  (*((volatile unsigned int*)0x400FF0D4))

/* ADC0 (base 0x4003B000) */
#define ADC0_SC1A   (*((volatile unsigned int*)0x4003B000))
#define ADC0_CFG1   (*((volatile unsigned int*)0x4003B008))
#define ADC0_SC2    (*((volatile unsigned int*)0x4003B020))
#define ADC0_RA     (*((volatile unsigned int*)0x4003B010))

#define ADC_COCO    (1u << 7)   /* Conversion Complete flag em SC1A */
#define ADC_CH8     8u          /* canal ADC0_SE8 = PTB0 */
#define THRESHOLD   2048u       /* limiar: metade de 4095 (escala 12 bits) */

#define LED_GREEN   (1u << 19)
#define LED_BLUE    (1u << 1)

int main(void) {
    /* (1) Habilitar clock das portas B (bit 10) e D (bit 12) */
    SIM_SCGC5 |= (1u << 10) | (1u << 12);

    /* (2) PTB0: MUX=000 (funcao analogica — desativa buffer digital) */
    PORTB_PCR0 = 0x00000000;

    /* Configurar pinos de LED como GPIO: MUX=001, bits [10:8] */
    PORTB_PCR19 = 0x00000100;  /* PTB19: LED verde */
    PORTD_PCR1  = 0x00000100;  /* PTD1:  LED azul  */

    /* (3) Definir pinos de LED como saida */
    GPIOB_PDDR |= LED_GREEN;
    GPIOD_PDDR |= LED_BLUE;

    /* Apagar ambos os LEDs (active-low: HIGH = apagado) */
    GPIOB_PSOR = LED_GREEN;
    GPIOD_PSOR = LED_BLUE;

    /* (4) Habilitar clock do modulo ADC0 (SIM_SCGC6 bit 27) */
    SIM_SCGC6 |= (1u << 27);

    /* (5) Trigger por software (ADTRG=0 em SC2) */
    ADC0_SC2 = 0x00;

    /* (6) Clock = bus clock, resolucao 12 bits single-ended
       ADC0_CFG1: bits [3:2] = MODE = 01 (12 bits) */
    ADC0_CFG1 = 0x04;

    for (;;) {
        /* (7) Iniciar conversao: escreve canal em SC1A (inicia e zera COCO) */
        ADC0_SC1A = ADC_CH8;

        /* (8) Aguardar fim de conversao (COCO = 1) */
        while (!(ADC0_SC1A & ADC_COCO)) {}

        /* (9) Ler resultado (0 = 0 V, 4095 = 3,3 V) */
        unsigned int result = ADC0_RA;

        /* (10) Acionar LED conforme tensao lida */
        if (result > THRESHOLD) {
            /* Proximo de 3,3 V: LED azul aceso, verde apagado */
            GPIOD_PCOR = LED_BLUE;
            GPIOB_PSOR = LED_GREEN;
        } else {
            /* Proximo de 0 V: LED verde aceso, azul apagado */
            GPIOB_PCOR = LED_GREEN;
            GPIOD_PSOR = LED_BLUE;
        }
    }

    return 0;
}
