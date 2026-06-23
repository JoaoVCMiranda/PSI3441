> [!IMPORTANT]
> João Victor Cavalcante Miranda (#14582927)
> https://github.com/JoaoVCMiranda/PSI3441/blob/15c4b1a2d90e16e07eb0269e0f36c6c9e9da74f1/entregas/8/src/main.c

## Atividade 8 — Aquisição de Dados em Tempo Real

Dois threads: `t_acquire` lê o ADC e enfileira amostras com `K_NO_WAIT`; `t_comm` retira da fila e transmite CSV pela UART. A comunicação entre eles é feita via `k_msgq`, que elimina o acesso compartilhado completamente — é a evolução natural do mutex da entrega 7.

O filtro FIR de 8 taps é implementado em ponto fixo (sem softfloat): 8 somas de inteiros e um shift de 3 bits no lugar da divisão por 8. O impacto na taxa de aquisição foi praticamente zero — o gargalo real é a UART.

A 115 200 bps, cada linha de ~22 bytes ocupa 1,9 ms de UART, limitando `t_comm` a ~524 Hz. O ADC converte em ~7 µs (potencial de 142 kHz). O processador passa a maior parte do tempo esperando a UART terminar de transmitir, não processando amostras.

**Problema observado na primeira execução: nenhuma saída no terminal.**

Na primeira tentativa não apareceu nenhuma saída. A causa provável foi um stack overflow silencioso em `t_acquire`: com `STACK_SIZE=1024`, o driver ADC (que usa IRQ + semáforo interno) ultrapassava a pilha, travando a thread sem nenhuma mensagem de erro. Com `t_acquire` travada, a fila `k_msgq` nunca recebe amostras e `t_comm` bloqueia para sempre no `k_msgq_get` — parecendo que nada acontece, mesmo que os três cabeçalhos `#` já tenham sido impressos antes do bloqueio.

A correção foi aumentar a pilha de `t_acquire` para 2048 bytes (`STACK_ACQ=2048`) e trocar `LOG_ERR` por `printk` na checagem de `device_is_ready`, para que o erro apareça mesmo se o backend de logging estiver com problema.
