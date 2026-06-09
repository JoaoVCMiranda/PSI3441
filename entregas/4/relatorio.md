> [!IMPORTANT]
> João Victor Cavalcante Miranda (#14582927)


## Atividade 4 — Aquisição ADC + controle de LEDs

Repo: https://github.com/JoaoVCMiranda/PSI3441/tree/main/4/main.c

O programa configura o ADC0 do FRDM-KL25Z para leitura analógica do pino PTB0 (canal SE8) e acende o LED azul (PTD1) para tensão próxima de 3,3 V ou o LED verde (PTB19) para tensão próxima de 0 V.

### Inicialização

1. **Clocks** — `SIM_SCGC5 |= (1<<10)|(1<<12)`: bit 10 = clock Porta B, bit 12 = clock Porta D. `SIM_SCGC6 |= (1<<27)`: bit 27 = clock módulo ADC0.
2. **PTB0 analógico** — `PORTB_PCR0 = 0x00`: MUX=000 desabilita o buffer digital do pino, habilitando a função analógica.
3. **LEDs como GPIO** — `PORTB_PCR19 = 0x100` e `PORTD_PCR1 = 0x100`: MUX=001 para GPIO. Direção setada com `GPIOB_PDDR |= (1<<19)` e `GPIOD_PDDR |= (1<<1)`.

### ADC0

Trigger por software: `ADC0_SC2 = 0x00`. Resolução 12 bits com bus clock: `ADC0_CFG1 = 0x04` (MODE bits [3:2] = `01`).

Escrever o número do canal em `ADC0_SC1A` inicia a conversão e zera o flag COCO. Aguarda-se o bit 7 (COCO) ser setado, então lê-se o resultado em `ADC0_RA` (0 = 0 V, 4095 aprox. 3,3 V).

```c
ADC0_SC1A = 8;                       /* canal SE8 = PTB0; inicia conversão */
// E legal fazer operaoes binarias com c
// jA que os estados de atividade sao salvos no stream de bits(nomeados LED_VERDE, LED_AZUL)
while (!(ADC0_SC1A & (1u << 7))) {}  /* aguarda COCO                       */
unsigned int result = ADC0_RA;
```

### Controle dos LEDs

Limiar em 2048 (metade da escala de 12 bits). PSOR/PCOR garantem operação atômica sobre PDOR — ver ex3:

```c
if (result > 2048) {
    GPIOD_PCOR = (1u << 1);   /* azul  aceso  */
    GPIOB_PSOR = (1u << 19);  /* verde apagado */
} else {
    GPIOB_PCOR = (1u << 19);  /* verde aceso  */
    GPIOD_PSOR = (1u << 1);   /* azul  apagado */
}
```
