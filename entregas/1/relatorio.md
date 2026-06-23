> [!IMPORTANT]
> João Victor Cavalcante Miranda (#14582927)

## Atividade 1 — Pisca LED

O programa pisca o LED vermelho integrado da FRDM-KL25Z a cada 500 ms usando a API GPIO do Zephyr.

### Diagrama de conexões

Sem conexões externas — usa o LED RGB integrado à placa.

| Pino (KL25Z) | Alias DTS | Cor | Ativo |
|---|---|---|---|
| PTB18 | led2 | Vermelho | LOW |

### Como usar

```bash
# Flash
pio run -t upload

# Monitor serial
pio device monitor --baud 115200
```

### Funcionamento

O Zephyr descreve o hardware da placa em um **Device Tree** (DTS). O alias `led2` já aponta para PTB18 com a flag `GPIO_ACTIVE_LOW`, então a API cuida da inversão de lógica automaticamente — `GPIO_OUTPUT_INACTIVE` deixa o LED apagado, `gpio_pin_toggle_dt` alterna o estado sem precisar saber se o pino é active-low ou active-high.

O loop principal alterna o LED e dorme 500 ms com `k_msleep()`. Durante o sleep o scheduler Zephyr pode rodar outras threads; o processador não fica em busy-wait.

---

## Apêndice — Conceitos relevantes

**Device Tree (DTS)**: arquivo que descreve o hardware para o Zephyr. O BSP do frdm_kl25z já define os três LEDs com seus pinos e polaridade. O código referencia pelo alias (`led2`) em vez de endereço direto, o que o torna portável.

**GPIO_ACTIVE_LOW**: indica que o pino acende o LED quando está em nível lógico 0. A API Zephyr inverte o nível internamente, então `gpio_pin_set_dt(&led, 1)` significa "ligar o LED", independente da polaridade do hardware.

**k_msleep()**: coloca a thread em sleep por N milissegundos e cede a CPU ao scheduler. Diferente de um loop `volatile` de espera, não desperdiça ciclos de processador.
