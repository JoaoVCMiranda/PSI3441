# Atividade 3 — Relatório

**PSI3441 — Arquitetura de Sistemas Embarcados**
**Plataforma:** FRDM-KL25Z (ARM Cortex-M0+, ~21 MHz)

---

## Objetivo

Fazer o LED verde (PTB19) piscar com período de 2 segundos, acessando registradores diretamente em C, sem uso do Processor Expert.

---

## Registradores Utilizados

| Registrador | Endereço | Uso |
|---|---|---|
| `SIM_SCGC5` | `0x40048038` | Habilitar clock da Porta B (bit 10) |
| `PORTB_PCR19` | `0x4004A04C` | Configurar mux do pino PTB19 como GPIO (MUX=001) |
| `GPIOB_PDDR` | `0x400FF054` | Definir PTB19 como saída (bit 19 = 1) |
| `GPIOB_PSOR` | `0x400FF044` | Setar pino HIGH → LED apagado (active-low) |
| `GPIOB_PCOR` | `0x400FF048` | Setar pino LOW  → LED aceso  (active-low) |

**Cálculo dos endereços:**
- Base Porta B (PCR): `0x4004A000` + `19 × 4` = `0x4004A04C`
- Base GPIO Porta B: `0x400FF040`; PDDR está no offset `+0x14`

---

## Função de Espera

```c
void delayMs(int n) {
    int i, j;
    for (i = 0; i < n; i++)
        for (j = 0; j < 7000; j++) {}
}
```

O loop interno com 7000 iterações foi calibrado para aproximadamente 1 ms ao clock default de ~21 MHz do KL25Z. Para período de 2 s usa-se `delayMs(1000)` em cada semiciclo.

---

## Sequência de Inicialização

1. `SIM_SCGC5 |= (1 << 10)` — habilita clock da Porta B
2. `PORTB_PCR19 = 0x00000100` — seta MUX=001 (bits [10:8]) para função GPIO
3. `GPIOB_PDDR |= (1 << 19)` — configura PTB19 como saída

---

## Loop Principal

```
loop:
    GPIOB_PCOR = (1<<19)   → LED aceso  (pino LOW)
    delayMs(1000)           → aguarda 1 s
    GPIOB_PSOR = (1<<19)   → LED apagado (pino HIGH)
    delayMs(1000)           → aguarda 1 s
    goto loop
```

O LED verde do FRDM-KL25Z é active-low: o anodo está ligado a 3,3 V e o catodo ao pino PTB19 via resistor. Portanto, pino LOW acende e pino HIGH apaga o LED.

---

## Arquivo

- `main.c` — código-fonte completo
