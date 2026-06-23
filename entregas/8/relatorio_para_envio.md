> [!IMPORTANT]
> João Victor Cavalcante Miranda (#14582927)
> https://github.com/JoaoVCMiranda/PSI3441/blob/15c4b1a2d90e16e07eb0269e0f36c6c9e9da74f1/entregas/8/src/main.c

## Atividade 8 — Aquisição de Dados em Tempo Real

Dois threads: `t_acquire` lê o ADC e enfileira amostras com `K_NO_WAIT`; `t_comm` retira da fila e transmite CSV pela UART. A comunicação entre eles é feita via `k_msgq`, que elimina o acesso compartilhado completamente — é a evolução natural do mutex da entrega 7.

O filtro FIR de 8 taps é implementado em ponto fixo (sem softfloat): 8 somas de inteiros e um shift de 3 bits no lugar da divisão por 8. O impacto na taxa de aquisição foi praticamente zero — o gargalo real é a UART.

A 115 200 bps, cada linha de ~22 bytes ocupa 1,9 ms de UART, limitando `t_comm` a ~524 Hz. O ADC converte em ~7 µs (potencial de 142 kHz). O processador passa a maior parte do tempo esperando a UART terminar de transmitir, não processando amostras.
