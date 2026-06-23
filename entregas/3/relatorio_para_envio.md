> [!IMPORTANT]
> João Victor Cavalcante Miranda (#14582927)
> https://github.com/JoaoVCMiranda/PSI3441/blob/c4babfcb33a30fb21b8aebb81ae317ed1b0a67de/entregas/3/src/main.c

## Atividade 3 — Pisca LED via registradores

Aqui tudo é feito na mão: habilitar o clock da porta B em `SIM_SCGC5`, configurar o MUX do pino via PCR, setar a direção no PDDR e depois alternar o estado no loop.

O ponto mais interessante foi entender por que usar `PSOR`/`PCOR` em vez de `PDOR`. Modificar o PDOR com `|=` ou `&=` é uma operação de leitura + escrita — se uma interrupção ocorrer no meio, o estado de outro pino pode ser corrompido. O PSOR e o PCOR são atômicos: escrever `1` no bit desejado afeta só aquele bit, sem ler o registrador inteiro.

A função `delayMs` com `j < 7000` foi calibrada para o clock default de ~21 MHz. No Zephyr, o clock sobe para 48 MHz antes de chamar o `main`, então o período real fica em torno de 860 ms em vez de 2 s.
