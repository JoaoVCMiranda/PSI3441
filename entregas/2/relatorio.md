> [!IMPORTANT]
> João Victor Cavalcante Miranda (#14582927)

## Atividade 2 — Controle Interativo de Cor e Intensidade (PWM + Terminal)

O programa controla o LED RGB integrado da FRDM-KL25Z via PWM usando a biblioteca `pwm_z42` (bare-metal TPM). O usuário digita letras no terminal serial para aumentar ou diminuir a intensidade de cada cor em passos de 10%.

### Diagrama de conexões

Todos os pinos são internos à placa (LED RGB embutido):

| Pino (KL25Z) | Função TPM | Canal | Cor | Ativo |
|---|---|---|---|---|
| PTB18 | TPM2_CH0 (ALT3) | CH0 | Vermelho | LOW |
| PTB19 | TPM2_CH1 (ALT3) | CH1 | Verde | LOW |
| PTD1  | TPM0_CH1 (ALT4) | CH1 | Azul | LOW |

### Como usar

```bash
# Flash
pio run -t upload

# Monitor serial (115 200 baud)
pio device monitor --baud 115200
# ou
cat /dev/ttyACM0
```

Ao conectar, o terminal exibe:

```
PSI3441 — Atividade 2: Controle de Cor via PWM
  r/R = vermelho -/+10%
  g/G = verde    -/+10%
  b/B = azul     -/+10%

R=  0%  G=  0%  B=  0%
```

Digite os caracteres diretamente no monitor. Cada tecla altera o canal em ±10% e exibe o estado atual.

### Funcionamento

#### TPM e clock

A biblioteca `pwm_z42` acessa os registradores TPM diretamente (bare-metal), da mesma forma que a Atividade 5. A fonte de clock escolhida é o **MCGIRCLK** (Fast IRC = 4 MHz), independente do PLL de 48 MHz que o Zephyr configura.

| Parâmetro | Valor | Resultado |
|---|---|---|
| Fonte | MCGIRCLK | 4 MHz |
| Prescaler | PS=64 | f_tpm = 62 500 Hz |
| MOD | 624 | f_pwm = **100 Hz** |

Com 100 Hz, o PWM está acima do limiar de fusão visual (~50 Hz), então o LED parece sólido mesmo em baixas intensidades.

#### Por que MCGIRCLK e não PLLFLL?

No Zephyr o PLL sobe para 48 MHz. Com PLLFLL e PS=128, o MOD necessário para frequências baixas ultrapassaria 16 bits (limite do registrador TPM). O MCGIRCLK resolve sem mudar a lógica da biblioteca.

#### Duty cycle e LED active-low

O modo `TPM_PWM_L` (low-true) produz:
- Saída **LOW** quando contador ≥ CnV
- Saída **HIGH** quando contador < CnV

O LED acende quando o pino está em LOW (active-low). Portanto:

```
brilho = CnV / MOD     (CnV = 0 → LED apagado, CnV = MOD → brilho máximo)
```

Cada nível (0–10) mapeia para `CnV = nível × MOD / 10`.

#### Leitura do terminal

`uart_poll_in()` lê diretamente do registrador de dados do UART0 (modo polling). Como o console Zephyr só usa `uart_poll_out()` para `printk`, não há conflito. Quando não há byte disponível, `k_sleep(K_MSEC(10))` cede a CPU ao scheduler.

#### Inicialização dos pinos

`pwm_tpm_Ch_Init()` escreve o campo MUX no registrador `PORT_PCR[pin]`:
- PTB18, PTB19 → `PORTB->PCR[pin] = PORT_PCR_MUX(3)` (ALT3 = TPM2)
- PTD1 → `PORTD->PCR[1] = PORT_PCR_MUX(4)` (ALT4 = TPM0)

O Zephyr já habilitou os clocks de PORTB e PORTD via `CONFIG_GPIO=y` antes de `main()` ser chamada.

---

## Apêndice — Conceitos relevantes

**TPM (Timer/PWM Module)**: periférico do KL25Z que gera sinais PWM por comparação entre um contador livre e o registrador `CnV`. O período é definido por `MOD`; o duty cycle, por `CnV`.

**MCGIRCLK**: oscilador interno de referência do MCG (Multipurpose Clock Generator). Opera a 4 MHz (Fast IRC) e é independente do PLL — útil quando o Zephyr sobe o sistema para 48 MHz e os valores de MOD para o PLLFLL não cabem em 16 bits.

**PORT_PCR MUX**: cada pino do KL25Z tem um campo de 3 bits no registrador PCR que seleciona a função elétrica: MUX=1 = GPIO, MUX=3 = ALT3 (ex.: TPM2), MUX=4 = ALT4 (ex.: TPM0 em PORTD).

**PWM low-true**: o sinal fica LOW durante a janela ativa. Combinado com um LED active-low, quanto maior o CnV, maior o brilho.

**uart_poll_in()**: API Zephyr que sonda o registrador de dados do UART sem usar interrupções. Retorna 0 se um byte foi lido, negativo se o buffer estava vazio.
