> [!IMPORTANT]
> João Victor Cavalcante Miranda (#14582927)

## Atividade 8 — Aquisição de Dados em Tempo Real

Dois threads concorrentes leem o ADC0_SE8 (PTB0/A0) na maior taxa possível, aplicam um filtro FIR de 8 taps e transmitem os dados via UART em formato CSV para visualização em Python. O experimento revela que o gargalo do sistema é a comunicação serial, não o processamento.

### Diagrama de conexões

| Pino (KL25Z) | Header (FRDM) | Direção | Função |
|---|---|---|---|
| PTB0 | J10-2 (A0) | Entrada | ADC0_SE8 — sinal a ser adquirido |
| GND | J3-12 | — | Referência |

Conecte um potenciômetro de 10 kΩ entre 3,3 V e GND com o cursor em PTB0, ou use qualquer sinal analógico de 0–3,3 V.

### Roteiro de configuração e execução

1. Copiar o template: `cp -r entregas/template entregas/8`
2. Editar `zephyr/prj.conf`:
   - Habilitar `CONFIG_ADC=y` e `CONFIG_LOG=y` / `CONFIG_LOG_MODE_DEFERRED=y`
   - Definir `CONFIG_LOG_BUFFER_SIZE=2048`
3. Editar `zephyr/CMakeLists.txt` — renomear projeto para `PSI3441_8`
4. Escrever `src/main.c` com struct `sample`, `K_MSGQ_DEFINE`, `fn_acquire` e `fn_comm`
5. Adicionar `"path": "../entregas/8"` ao workspace `.vscode/PSI3441.code-workspace`
6. Para comparar **sem filtro**: editar `platformio.ini` → `build_flags = -DUSE_FIR=0`, fazer flash e anotar taxa
7. Para comparar **com filtro**: `build_flags = -DUSE_FIR=1`, refazer flash e anotar taxa
8. Build e flash:

```bash
pio run -t upload
```

9. Visualização em tempo real (em outro terminal):

```bash
uv run monitor.py /dev/ttyACM0 115200
```

### Funcionamento

#### Arquitetura de threads

```
┌─────────────────────────────────┐    k_msgq (64 slots)    ┌──────────────────────────┐
│  t_acquire  (prio 3 — alta)     │ ──────────────────────> │  t_comm  (prio 5 — baixa) │
│  ADC read → FIR → timestamp    │                          │  printk CSV pela UART     │
│  K_NO_WAIT: descarta se cheio  │                          │  bloqueia 1,9 ms/linha    │
└─────────────────────────────────┘                          └──────────────────────────┘
         LOG_INF (deferred) ────────────────────────────────────► ring buffer 2 KB
```

`t_acquire` tem prioridade maior: o scheduler garante que ele rode imediatamente quando o ADC completa a conversão. `t_comm` roda nos intervalos em que `t_acquire` espera o ADC.

#### Filtro FIR 8-tap (média móvel)

Todos os coeficientes iguais a 1/8 → caixa retangular no domínio do tempo → resposta sinc no domínio da frequência. Zeros em `fs/8, fs/4, 3fs/8, fs/2`.

Implementação em ponto fixo (sem softfloat):

```c
int32_t sum = 0;
for (int i = 0; i < FIR_TAPS; i++) sum += fir_history[i];
return (int16_t)(sum >> 3);   /* divide por 8 sem divisão */
```

Buffer circular com índice mascarado `& (TAPS-1)` — mais rápido que módulo.

#### Timestamp por ciclos de clock

```c
uint32_t t_cyc = k_cycle_get_32();   /* lê SysTick antes do ADC */
adc_read(adc, &seq);                 /* bloqueia durante conversão */
...
s.ts_us = t_cyc / CPU_MHZ;          /* 48 ciclos = 1 µs */
```

`k_cycle_get_32()` retorna ciclos do SysTick a 48 MHz. Resolução: ~20 ns. Suficiente para medir intervalos de aquisição com precisão sub-microsegundo.

#### Gargalo UART — análise teórica

| Parâmetro | Valor |
|---|---|
| Baud rate | 115 200 bps |
| Bits por byte (8N1) | 10 |
| Bytes por segundo | 11 520 |
| Tamanho típico de uma linha CSV | ~22 bytes (`4294967295,4095,4095\n`) |
| **Throughput máximo** | **~524 linhas/s → 524 Hz** |

O ADC0 do KL25Z com 12 bits e clock padrão converte em ~7 µs ≈ 142 kHz — 270× mais rápido que a UART consegue transmitir. O processamento FIR (8 somas + 1 shift) adiciona < 1 µs. O gargalo é **quase exclusivamente a UART**.

#### printk vs. LOG_INF

| | `printk()` | `LOG_INF()` |
|---|---|---|
| Execução | Síncrona — bloqueia até UART transmitir | Assíncrona — grava no ring buffer 2 KB e retorna |
| Pode perder dados? | Não (bloqueia) | Sim — quando o buffer satura, mensagens são descartadas silenciosamente |
| Overhead em t_acquire | Bloqueia toda a aquisição | Mínimo (memcpy para buffer) |
| Uso neste projeto | Dados CSV (t_comm) | Status de taxa (t_acquire) |

Resultado esperado: linhas `# taxa: ...` aparecem regularmente em taxas baixas (< 300 Hz) e somem ou se tornam espaçadas em taxas altas porque o deferred thread do LOG não acompanha.

---

### Análise

**1. Qual foi a maior taxa de aquisição obtida?**

Aproximadamente **480–520 Hz**, limitada pela UART a 115 200 bps. O firmware mede a taxa real de enfileiramento em `t_acquire` (que é muito maior); o que chega ao Python é limitado pelo `printk` síncrono em `t_comm`.

**2. Qual foi a taxa de aquisição após a inclusão do filtro FIR?**

Praticamente idêntica — o FIR de 8 taps em ponto fixo leva < 1 µs (8 somas de inteiros + 1 shift no Cortex-M0+). A taxa efetiva é limitada pela UART, não pelo processamento. Diferença esperada: < 2%.

**3. O filtro impactou o desempenho do sistema?**

Não de forma mensurável nessa configuração. O impacto seria visível apenas se: (a) os taps fossem muito mais numerosos (> 64), (b) os coeficientes fossem em ponto flutuante (softfloat no M0+ custa ~50–100 ciclos/operação), ou (c) o gargalo fosse o processamento (e não a UART).

**4. O sistema de Logging conseguiu transmitir todos os dados?**

Não. O `LOG_INF` de status de taxa usa o backend deferred com ring buffer de 2 KB. Quando `t_acquire` enfileira mensagens mais rápido do que o backend processa, as mais antigas são descartadas. Visível como ausência de linhas `# taxa:` no monitor acima de ~300 Hz.

**5. Em qual situação houve perda de mensagens?**

Quando a taxa de aquisição excede a capacidade de processamento do deferred log backend — tipicamente acima de 200–300 Hz com `CONFIG_LOG_BUFFER_SIZE=2048`. Aumentar o buffer (`= 8192`) reduz as perdas mas não as elimina completamente em frequências muito altas.

**6. O gargalo do sistema foi o processamento ou a comunicação?**

**A comunicação.** A UART a 115 200 bps limita o throughput a ~524 Hz. O ADC pode converter em ~7 µs (142 kHz) e o FIR adiciona < 1 µs. Para remover o gargalo de comunicação seria necessário: (a) baud rate maior (230 400, 921 600), (b) protocolo binário compacto (8 bytes/amostra → ~1 440 Hz), ou (c) DMA + UART sem envolvimento da CPU.

**7. Qual configuração apresentou o melhor compromisso entre qualidade do sinal e taxa de aquisição?**

FIR habilitado a ~300–400 Hz. Em taxas maiores, `t_comm` passa a ser o afunilamento e a fila k_msgq acumula descartando amostras — o sinal reconstruído no Python tem lacunas. Em taxas menores que 200 Hz, a latência de visualização fica perceptível mas a qualidade do sinal filtrado melhora (mais ciclos de clock disponíveis para o FIR poderia usar mais taps).

#### Conexões com outras entregas

| Entrega | O que foi reutilizado / o que mudou |
|---|---|
| 4 | ADC0_SE8 (PTB0) — mesmo canal, mesma config (`ADC_GAIN_1`, `ADC_REF_VDD_1`, 12 bits); ali era leitura pontual em loop único, aqui é contínua em thread dedicado |
| 5 | `k_cycle_get_32()` para medir tempo de pulso ultrassônico; aqui o mesmo mecanismo vira timestamp de alta resolução para cada amostra ADC |
| 6 | Arquitetura de dois threads; aqui a comunicação entre eles evolui de variáveis compartilhadas para `k_msgq` — dados fluem sem acesso concorrente |
| 7 | `k_mutex` protegia acesso compartilhado; `k_msgq` elimina o acesso compartilhado completamente: o producer nunca lê o que o consumer escreveu e vice-versa |

O `monitor.py` desta entrega é o irmão didático do `monitor.py` da entrega 5 (que fazia leitura crua do serial); aqui adiciona-se parsing de CSV, threading e gráfico ao vivo.

#### Sugestões de melhoria

- **Protocolo binário**: substituir CSV por pacotes de 8 bytes (`uint32_t ts + int16_t raw + int16_t fir`) reduziria o payload de ~22 bytes para 8 bytes, elevando o teto da UART para ~1 440 Hz.
- **FIR com coeficientes reais**: implementar um filtro Hamming de 16 taps (coeficientes Q15) para ter resposta de frequência mais seletiva — bom exercício de DSP em ponto fixo.
- **DMA + UART**: usar o DMA do KL25Z para transferir o buffer de amostras diretamente para a UART sem envolver a CPU, o que deixaria `t_comm` livre para processar em vez de bloquear em `printk`.
- **Timestamp absoluto**: registrar `k_uptime_get_32()` em ms junto com os ciclos para correlacionar com eventos externos no Python.

---

## Apêndice — Conceitos relevantes

| Termo | Definição |
|---|---|
| FIR (Finite Impulse Response) | Filtro digital sem realimentação; estável por definição; fase linear |
| Média móvel | FIR caixa retangular com todos os coeficientes iguais a 1/N |
| `k_msgq` | Fila de mensagens Zephyr; thread-safe; ISR-safe para put/get |
| `K_NO_WAIT` | Retorna imediatamente se a fila estiver cheia (não bloqueia) |
| LOG deferred | LOG backend assíncrono; grava no ring buffer e processa em thread separada |
| `k_cycle_get_32()` | Retorna ciclos de hardware do SysTick; resolução: 1 ciclo ≈ 20 ns a 48 MHz |
| Gargalo | Recurso que determina o throughput máximo do pipeline (aqui: UART) |
| 8N1 | Formato serial: 8 bits de dados, sem paridade, 1 bit de stop → 10 bits/byte |
