> [!IMPORTANT]
> João Victor Cavalcante Miranda (#14582927)

## Atividade 3 — LED verde piscando com período de 2 s (registradores)

O programa configura o LED verde do FRDM-KL25Z (PTB19, active-low) para piscar com período de 2 s acessando os registradores do hardware diretamente, sem nenhuma camada de abstração.

### Diagrama de conexões

Sem conexões externas — usa o LED RGB integrado à placa.

| Pino (KL25Z) | Função | Ativo |
|---|---|---|
| PTB19 | LED verde | LOW |

### Como usar

```bash
pio run -t upload
```

### Funcionamento

#### Sequência de inicialização

**(1) Clock da porta B** — `SIM_SCGC5 |= (1 << 10)`

O registrador `SIM_SCGC5` controla quais periféricos recebem clock. O bit 10 corresponde à Porta B. Sem habilitar esse clock, qualquer escrita nos registradores PCR ou GPIO dessa porta não tem efeito.

**(2) Função do pino** — `PORTB_PCR19 = 0x100`

Cada pino tem um registrador PCR (Pin Control Register). Os bits [10:8] selecionam a função elétrica (MUX). `001` (= `0x100`) seleciona GPIO. O padrão de reset é `000` (pino desconectado).

**(3) Direção** — `GPIOB_PDDR |= (1u << 19)`

`PDDR` (Port Data Direction Register) define quais pinos são entrada (0) e quais são saída (1). O `|=` preserva os demais bits.

#### Loop principal

| Passo | Registrador | Ação |
|---|---|---|
| Liga LED | PCOR (Clear) | Escreve `1` no bit 19 → pino vai para LOW |
| Apaga LED | PSOR (Set) | Escreve `1` no bit 19 → pino vai para HIGH |

**Por que PSOR/PCOR em vez de PDOR?**

Modificar `PDOR` diretamente exige leitura + modificação + escrita (`|=` / `&=`). Se uma interrupção ocorrer entre a leitura e a escrita, o estado de outro pino pode ser sobrescrito. `PSOR` e `PCOR` são atômicos: escrever `1` no bit desejado afeta apenas aquele bit — escrever `0` não faz nada.

#### Função de espera

```c
void delayMs(int n) {
    int i, j;
    for (i = 0; i < n; i++)
        for (j = 0; j < 7000; j++) {}
}
```

O loop interno com `j < 7000` foi calibrado para ~1 ms ao clock default do KL25Z (~21 MHz). Para período de 2 s: `delayMs(1000)` em cada semiciclo.

Em contexto Zephyr (48 MHz), o período real fica ~860 ms. Para corrigir: `j < 16000` (proporcional a 48/21).

---

## Apêndice — Conceitos relevantes

**SIM_SCGC5** (System Integration Module — System Clock Gating Control 5): registrador que liga e desliga o clock de periféricos individualmente para economizar energia. Endereço: `0x40048038`.

**PCR** (Pin Control Register): cada pino tem o seu. O campo MUX [10:8] seleciona entre as funções disponíveis: `000` = desconectado, `001` = GPIO, `010`–`111` = funções alternativas (UART, SPI, TPM…).

**PDDR / PDOR / PSOR / PCOR**: quatro registradores GPIO por porta. PDDR = direção; PDOR = estado atual (leitura/escrita completa); PSOR = seta bits atomicamente; PCOR = limpa bits atomicamente.

**Active-low**: o LED acende quando o pino está em nível lógico 0 (LOW). O cátodo do LED está ligado ao GPIO; o ânodo vai para VCC pelo resistor da placa.
