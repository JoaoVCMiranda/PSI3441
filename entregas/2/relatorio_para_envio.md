> [!IMPORTANT]
> João Victor Cavalcante Miranda (#14582927)
> https://github.com/JoaoVCMiranda/PSI3441/blob/ba43ffa8f3c67503040c400fa350b768fd3cbabf/entregas/2/src/main.c

## Atividade 2 — Controle Interativo de Cor e Intensidade (PWM + Terminal)

O controle dos LEDs RGB é feito via `pwm_z42`, que acessa os registradores TPM diretamente. A API Zephyr de PWM não expõe o registrador `MOD` em runtime, então o acesso bare-metal é necessário pelo mesmo motivo da atividade 5.

O clock escolhido foi o MCGIRCLK (4 MHz) em vez do PLLFLL porque o Zephyr eleva o PLL para 48 MHz — com isso, os valores de `MOD` para frequências baixas ultrapassariam 16 bits. Com MCGIRCLK e PS=64 chega-se a 100 Hz de PWM, acima do limiar de fusão visual.

Para leitura do terminal, `uart_poll_in` funciona junto com `printk` sem conflito porque o console Zephyr só usa a saída do UART (poll_out), deixando a entrada livre para leitura manual.
