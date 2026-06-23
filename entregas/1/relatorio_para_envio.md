> [!IMPORTANT]
> João Victor Cavalcante Miranda (#14582927)
> https://github.com/JoaoVCMiranda/PSI3441/blob/abea0ebaddf84092729b42c03cbea9cc7c303503/entregas/1/src/main.c

## Atividade 1 — Pisca LED

O ponto que mais me chamou atenção foi o Device Tree. Em vez de escrever o endereço do pino e tratar o active-low manualmente (como na atividade 3), o alias `led2` já carrega essas informações — o `gpio_pin_toggle_dt` inverte o estado sem precisar saber a polaridade do hardware.

O `k_msleep(500)` também é diferente do `delayMs` bare-metal: durante o sleep o scheduler pode rodar outras coisas. O processador não fica travado esperando.
