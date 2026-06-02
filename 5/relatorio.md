> [!IMPORTANT]
> João Victor Cavalcante Miranda (#14582927)

## Atividade 5 — PWM + Medida de Distância HC-SR04

Repo: https://github.com/JoaoVCMiranda/PSI3441/tree/main/5/main.c

O programa configura o TPM0 do FRDM-KL25Z para gerar um sinal PWM em PTC1 e usa o TPM1 como cronômetro de microsegundos para medir a distância via HC-SR04.

> **Sem bibliotecas.** O código não possui nenhum `#include`. Todos os periféricos são acessados por `#define` que mapeiam ponteiros `volatile` diretamente nos endereços dos registradores — a mesma abordagem das atividades 3 e 4. O "driver de PWM" é o próprio TPM configurado bit a bit.

### Diagrama de conexões

#### Pinagem utilizada

| Pino (KL25Z) | Header (FRDM) | Direção | Conectado a |
|---|---|---|---|
| PTC1 | J1-1 | Saída (PWM) | Canal CH1 do osciloscópio / carga |
| PTA1 | J2-20 | Saída (TRIG) | HC-SR04 pino TRIG |
| PTA2 | J2-18 | Entrada (ECHO) | Divisor resistivo → HC-SR04 pino ECHO |
| 3V3 | J3-4 | — | (referência de tensão) |
| 5V | J3-10 | — | HC-SR04 pino VCC |
| GND | J3-12 | — | HC-SR04 pino GND |

#### Esquema de ligação

```
FRDM-KL25Z                        HC-SR04
┌─────────────┐                  ┌──────────┐
│         PTA1├─────────────────►│TRIG      │
│             │                  │          │
│         PTA2├──┐          ┌───►│ECHO  5 V output
│             │  │R1 1kΩ    │    │          │
│          GND├──┤          └────┤          │
│             │  │R2 2kΩ        │          │
│             │  └──────────────►GND        │
│          5 V├───────────────── ►VCC       │
└─────────────┘                  └──────────┘
```

**Por que o divisor resistivo em ECHO?**
O HC-SR04 alimentado a 5 V produz um pulso ECHO de 5 V, mas os pinos GPIO do KL25Z suportam no máximo VDD + 0,3 V ≈ 3,6 V. Sem proteção, 5 V danificaria o pino. O divisor R1 = 1 kΩ / R2 = 2 kΩ reduz para:

```
V_PTA2 = 5 V × 2000 / (1000 + 2000) = 3,33 V  ✓
```

O pino TRIG pode ser conectado diretamente: o KL25Z entrega 3,3 V como HIGH, e o limiar de acionamento do HC-SR04 é ≈ 2 V.

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

---

## Dicionário de Siglas e Conceitos

### Siglas de módulos e registradores

**SIM — System Integration Module**
Módulo central de configuração do chip. Concentra registradores que controlam clock gating, seleção de fontes de clock e mapeamento de sinais de periféricos para pinos.

**SCGC — System Clock Gating Control**
Registradores dentro do SIM (`SCGC5`, `SCGC6`) que habilitam ou desabilitam o clock de cada periférico individualmente. Por padrão todos os periféricos nascem sem clock para economizar energia — é preciso setar o bit correspondente antes de acessar qualquer registrador do periférico; caso contrário a leitura/escrita não tem efeito (ou causa hard fault).

```c
SIM_SCGC5 |= (1u << 9) | (1u << 11);  /* habilita clock de PORTA e PORTC */
SIM_SCGC6 |= (1u << 24) | (1u << 25); /* habilita clock de TPM0 e TPM1   */
```

**SOPT — System Options Register**
Registradores de opções do sistema. `SIM_SOPT2` contém o campo `TPMSRC` (bits [25:24]) que escolhe a fonte de clock dos módulos TPM.

**MCG — Multipurpose Clock Generator**
Módulo gerador de clocks do KL25Z. Contém o FLL, o PLL e os osciladores internos. Os registradores de controle `MCG_C1` e `MCG_C2` configuram quais osciladores ficam ativos e como o clock do núcleo é derivado.

**FLL — Frequency-Locked Loop**
Malha de travamento de frequência interna do MCG. No estado padrão (FEI — FLL Engaged Internal) usa o oscilador lento de 32 kHz como referência e multiplica para ~20,97 MHz. Neste código não é usado como fonte dos TPMs — optamos pelo IRC para ter frequência exata.

**IRC — Internal Reference Clock**
Oscilador RC interno do MCG. Tem dois modos selecionados por `MCG_C2[IRCS]`:
- `IRCS=0`: Slow IRC (~32 kHz)
- `IRCS=1`: Fast IRC (4 MHz, trimado em fábrica)

`MCG_C1[IRCLKEN]` habilita o IRC independente do modo de clock do núcleo, tornando-o disponível como `MCGIRCCLK`.

**MCGIRCCLK**
Nome do sinal de saída do IRC no barramento de clocks do chip. Selecionado como fonte dos TPMs via `SIM_SOPT2[TPMSRC]=11`.

**TPM — Timer/PWM Module**
Módulo timer/contador com canais que podem operar em modo captura de entrada, comparação de saída ou PWM. O KL25Z tem TPM0 (6 canais), TPM1 e TPM2 (2 canais cada). Cada instância tem:

| Registrador | Função |
|---|---|
| `TPM_SC` | Status e controle: prescaler (`PS`), fonte de clock (`CMOD`), modo central (`CPWMS`) |
| `TPM_CNT` | Contador atual (escrever qualquer valor reseta para 0) |
| `TPM_MOD` | Módulo: valor máximo do contador; define o período do PWM |
| `TPM_CnSC` | Controle do canal n: modo (`MSA`/`MSB`) e polaridade (`ELSA`/`ELSB`) |
| `TPM_CnV` | Valor do canal n: define largura de pulso (duty cycle) no modo PWM |

**PS — Prescaler**
Divisor de frequência aplicado antes do contador do TPM. Configurado nos bits [2:0] de `TPM_SC`. Valores: 0=÷1, 1=÷2, 2=÷4, 3=÷8, 4=÷16, 5=÷32, 6=÷64, 7=÷128. Permite ajustar a resolução e o range do timer.

**CMOD — Clock Mode Selection**
Bits [4:3] de `TPM_SC`. Controlam quando o contador incrementa:
- `00`: TPM desabilitado
- `01`: incrementa a cada ciclo do clock selecionado (modo normal)
- `10`: incrementa na borda de subida do clock externo

**MOD — Modulo**
Valor em `TPM_MOD` que define o teto do contador. Quando `CNT` atinge `MOD`, ele reseta para 0 no próximo ciclo. O período do PWM é `(MOD + 1) / f_tpm`.

**MSB / MSA — Mode Select B / A**
Bits 5 e 4 de `TPM_CnSC`. A combinação `MSB=1, MSA=x` coloca o canal em modo PWM. `MSB=0, MSA=1` é modo de comparação de saída.

**ELSB / ELSA — Edge or Level Select B / A**
Bits 3 e 2 de `TPM_CnSC`. No modo PWM edge-aligned definem a polaridade:
- `ELSB=1, ELSA=0`: **high-true** — pino começa HIGH, vai LOW quando `CNT == CnV`, volta HIGH quando `CNT == MOD`
- `ELSB=0, ELSA=1`: low-true (polaridade invertida)

**PORT / PCR — Port Control Register**
Cada porta GPIO (A–E) tem um módulo PORT com um `PCR` de 32 bits por pino. O campo `MUX` (bits [10:8]) escolhe qual função o pino exerce:

| MUX | Função |
|---|---|
| `000` | Analógico (desabilita buffer digital) |
| `001` | GPIO |
| `010`–`111` | Funções alternativas de periféricos (ALT2…ALT7) |

Neste código `PTC1` recebe `MUX=4` (ALT4 = `TPM0_CH0`) e `PTA1`/`PTA2` recebem `MUX=1` (GPIO).

**GPIO — General Purpose Input/Output**
Módulo de entrada/saída digital de propósito geral. Cada porta tem seis registradores de 32 bits, um bit por pino:

| Registrador | Operação |
|---|---|
| `PDDR` | Data Direction Register — `1` = saída, `0` = entrada |
| `PDOR` | Data Output Register — estado atual de saída (leitura/escrita direta) |
| `PSOR` | Port Set Output — escrever `1` no bit N força pino N para HIGH; `0` não tem efeito |
| `PCOR` | Port Clear Output — escrever `1` no bit N força pino N para LOW; `0` não tem efeito |
| `PTOR` | Port Toggle Output — escrever `1` inverte o pino; `0` não tem efeito |
| `PDIR` | Port Data Input — leitura do estado físico dos pinos |

`PSOR`/`PCOR` são preferíveis a `PDOR |= mask` / `PDOR &= ~mask` porque a operação é **atômica**: não envolve leitura-modificação-escrita, então uma interrupção entre os passos não pode corromper outros pinos.

**MUX — Multiplexer**
No contexto de pinos, o MUX é o seletor de função descrito no campo `PCR[MUX]`. O pino físico é compartilhado por vários periféricos internos; o MUX conecta o pino ao periférico desejado.

---

### Conceitos gerais

**Máscara de bits (bit mask)**
Valor inteiro usado com operadores bitwise para isolar, setar ou limpar bits específicos de um registrador sem afetar os demais.

```c
/* Setar o bit 10 sem alterar os outros bits de SIM_SCGC5 */
SIM_SCGC5 |= (1u << 10);   /* OR com máscara: seta apenas o bit 10       */

/* Limpar o bit 10 */
SIM_SCGC5 &= ~(1u << 10);  /* AND com complemento: zera apenas o bit 10  */

/* Ler o bit 10 */
if (SIM_SCGC5 & (1u << 10)) { ... }  /* AND com máscara: isola o bit      */
```

`(1u << N)` cria uma máscara com apenas o bit N em 1. O sufixo `u` garante que o deslocamento opere sobre `unsigned int`, evitando comportamento indefinido para deslocamentos no bit de sinal.

**Operador `|=` (OR de atribuição)**
`A |= mask` equivale a `A = A | mask`. Seta todos os bits que estão em 1 na máscara, preservando os demais. Usado para habilitar bits em registradores de configuração.

**Operador `&= ~` (AND com complemento)**
`A &= ~mask` equivale a `A = A & (~mask)`. Zera todos os bits que estão em 1 na máscara, preservando os demais. Usado para limpar bits em registradores.

**`volatile`**
Qualificador de tipo do C que instrui o compilador a **nunca otimizar** acessos à variável — toda leitura vai ao endereço de memória, toda escrita é emitida imediatamente. Obrigatório em ponteiros para registradores mapeados em memória (MMIO): sem `volatile` o compilador pode eliminar leituras "redundantes" ou reordenar escritas, corrompendo a comunicação com o hardware.

```c
/* Sem volatile: o compilador pode remover o loop inteiro */
while (*(unsigned int*)0x40038004 < 499) {}

/* Com volatile: cada iteracao le o registrador de verdade */
#define TPM0_CNT (*((volatile unsigned int*)0x40038004))
while (TPM0_CNT < 499) {}
```

**Clock gating**
Técnica de economia de energia em que o clock de um periférico é bloqueado (gate = portão) quando ele não está em uso. No KL25Z o clock gating é controlado pelos registradores `SIM_SCGCx`. Acessar um periférico sem habilitar seu clock resulta em operação silenciosamente incorreta ou hard fault.

**Prescaler**
Divisor de frequência colocado antes de um contador ou periférico. No TPM o prescaler divide o clock de entrada antes de incrementar `TPM_CNT`, aumentando o período máximo mensurável à custa de resolução. Com clock de 4 MHz e PS=÷8 → resolução de 2 µs por tick; com PS=÷4 → 1 µs por tick.

**Edge-aligned PWM**
Modo em que o contador sobe de 0 até `MOD` e reinicia. O pino sai HIGH no início do período e vai LOW quando o contador atinge `CnV` (ou o inverso, dependendo de `ELSA`/`ELSB`). Resulta em pulsos alinhados pela borda inicial — adequado para controle de duty cycle.

```
CNT:  0 ──────── CnV ──── MOD ── 0 ────── ...
Pino: ╔══════════╗              ╔═════
      HIGH       ╚══════════════╝ LOW
                 |←── duty ──→|
```

**Duty cycle**
Fração do período em que o sinal permanece em nível alto: `duty = CnV / (MOD + 1)`. No código: `CnV = 249`, `MOD = 499` → duty = 250/500 = 50 %.

**Rising / falling edge**
- *Rising edge* (borda de subida): transição do sinal de LOW para HIGH.
- *Falling edge* (borda de descida): transição de HIGH para LOW.
Na medição do HC-SR04 aguarda-se a rising edge do ECHO para iniciar a contagem e a falling edge para encerrá-la.
