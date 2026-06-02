> [!IMPORTANT]
> João Victor Cavalcante Miranda (#14582927)

## Atividade 5 — PWM + Medida de Distância HC-SR04

Repo: https://github.com/JoaoVCMiranda/PSI3441/tree/main/5/main.c

O programa configura o TPM0 do FRDM-KL25Z para gerar um sinal PWM em PTC1 e usa o TPM1 como cronômetro de microsegundos para medir a distância via HC-SR04.

### Fonte de clock dos TPMs

O Fast IRC interno gera 4 MHz (`MCG_C2[IRCS]=1`, `MCG_C1[IRCLKEN]=1`). `SIM_SOPT2[TPMSRC]=11` seleciona `MCGIRCCLK` como entrada dos módulos TPM, garantindo frequência conhecida independente da configuração do FLL.

### TPM0 — geração de PWM (1 kHz, 50 %)

```
MCGIRCCLK (4 MHz) / PS=÷8 → 500 kHz
MOD = 499 → período = 500 ciclos / 500 kHz = 1 ms = 1 kHz
C0V = 249 → duty = 250/500 = 50 %
```

`TPM0_C0SC`: `MSB=1` (modo PWM), `ELSB=1` (pulso high-true edge-aligned). O pino PTC1 recebe `MUX=4` (ALT4 = `TPM0_CH0`).

### TPM1 — cronômetro de microsegundos

```
MCGIRCCLK (4 MHz) / PS=÷4 → 1 MHz → 1 tick = 1 µs
MOD = 0xFFFF → contador livre até 65,535 ms
```

Escrever qualquer valor em `TPM1_CNT` reinicia o contador a zero. A função `delayUs(n)` reseta o contador e aguarda `TPM1_CNT < n`.

### HC-SR04 — sequência trigger/echo

1. `TRIG` (PTA1) em HIGH por 10 µs → sensor emite rajada ultrassônica.
2. `ECHO` (PTA2) sobe para HIGH quando o sinal sai e desce quando o eco retorna.
3. A duração do pulso em `ECHO` é o tempo de ida e volta, em µs.

```c
GPIOA_PSOR = TRIG;
delayUs(10);
GPIOA_PCOR = TRIG;
// aguarda rising edge, mede ate falling edge
```

Timeout de 30 ms cobre o range máximo do sensor (~5 m).

### Cálculo de distância

```
v_som ≈ 340 m/s = 34000 cm/s
d = t × v / 2  (ida e volta)
d_cm = t_µs × 34000 / 2 / 1000000 = t_µs / 58,8 ≈ t_µs / 58
```

O resultado `d_cm` é salvo em uma variável `volatile` para leitura via debugger com breakpoint.
