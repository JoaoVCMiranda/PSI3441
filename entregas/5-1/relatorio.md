> [!IMPORTANT]
> João Victor Cavalcante Miranda (#14582927)

## Atividade 5-1 — HC-SR04 + PWM radar (Zephyr híbrido)

Versão Zephyr da entrega 5. Mantém o mesmo circuito (HC-SR04 em PTC8/PTC9, LED verde PTB19 via TPM2_CH1), mas substitui toda a inicialização manual por APIs do Zephyr onde possível.

| O que mudou | Bare-metal (5) | Zephyr (5-1) |
|---|---|---|
| GPIO TRIG/ECHO | SCGC5 + PCR manual | `gpio_pin_configure()` |
| Delay de trigger | loop `volatile` calibrado | `k_busy_wait(10)` |
| Debug serial | `uart_init()` + `uart_putc()` | `printk()` |
| Delay entre medições | loop `volatile` ~60 ms | `k_sleep(K_MSEC(60))` |
| PWM (TPM2) | `pwm_z42` | `pwm_z42` (igual — API Zephyr não expõe MOD runtime) |
| Leitura do echo | `GPIOC->PDIR` direto | `GPIOC->PDIR` direto (overhead do driver inaceitável) |

### Por que TPM2->MOD continua bare-metal

A API Zephyr `pwm_set_cycles()` altera apenas `CnV` (duty cycle). Não há como mudar o período (`MOD`) em runtime via driver padrão. Para o efeito radar — frequência de piscar proporcional à distância — é necessário escrever `TPM2->MOD` diretamente.

### Clock do TPM2

Bare-metal usava `PLLFLL` (~21 MHz). No Zephyr o PLL está em 48 MHz; com PS=128 o `MOD_FAR` excederia 16 bits. Solução: `MCGIRCLK` (Fast IRC = 4 MHz, independente do PLL), PS=64 → 62 500 Hz.

| | Bare-metal | Zephyr |
|---|---|---|
| Fonte | PLLFLL ~21 MHz | MCGIRCLK 4 MHz |
| PS | 128 | 64 |
| f_tpm | ~164 kHz | 62 500 Hz |
| MOD_NEAR (~10 Hz) | 16 000 | 6 250 |
| MOD_FAR (~2.7 Hz) | 60 000 | 23 150 |

### Conexões (idênticas à entrega 5)

| Pino (KL25Z) | Header (FRDM) | Direção | Função |
|---|---|---|---|
| PTB19 | J2-2 | Saída PWM | LED verde integrado (active-low) |
| PTC8 | J1-9 | Saída | HC-SR04 TRIG |
| PTC9 | J1-10 | Entrada | HC-SR04 ECHO |
| 5V | J3-10 | — | HC-SR04 VCC |
| GND | J3-12 | — | HC-SR04 GND |
