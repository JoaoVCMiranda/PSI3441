> [!IMPORTTANT]
> João Victor Cavalcante Miranda (#14582927)

<!-- respostas comentários e análises !-->

## Atividade 3 — LED verde piscando com período de 2 s

Código disponível em: https://github.com/JoaoVCMiranda/PSI3441/tree/main/3/main.c

O programa configura o LED verde do FRDM-KL25Z (PTB19, active-low) para piscar com período de 2 s manipulando registradores diretamente, sem bibliotecas de abstração.

### Inicialização

1. **Clock da porta B** — `SIM_SCGC5 |= (1 << 10)`: o operador `|=` preserva os bits já setados no registrador; `1 << 10` desloca o valor `1` dez posições para a esquerda, atingindo exatamente o bit que controla o clock da Porta B.
2. **MUX do pino** — `PORTB_PCR19 = 0x100`: bits [10:8] = `001` seleciona a função GPIO.
3. **Direção** — `GPIOB_PDDR |= (1 << 19)`: seta o bit 19 como saída.

### PDOR, PSOR e PCOR

`PDOR` (Port Data Output Register) guarda o estado atual de cada pino. Modificá-lo com `|=` / `&=` exige uma leitura seguida de escrita — se uma interrupção ocorrer no meio, outro pino pode ser corrompido.

`PSOR` e `PCOR` resolvem isso: escrever `1` em um bit do **PSOR** seta aquele bit no PDOR (pino HIGH); escrever `1` no **PCOR** limpa aquele bit (pino LOW). Escrever `0` não tem efeito sobre nenhum outro bit — a operação é atômica.

```c
GPIOB_PCOR = (1u << 19);  /* pino LOW  → LED aceso  */
GPIOB_PSOR = (1u << 19);  /* pino HIGH → LED apagado */
```

### Função de espera

A função `delayMs(n)` usa dois loops aninhados. O loop interno com `j < 7000` foi calibrado para aproximadamente 1 ms ao clock padrão do KL25Z (~21 MHz). Para o período de 2 s: `delayMs(1000)` em cada semiciclo.
