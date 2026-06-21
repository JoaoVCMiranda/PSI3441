> [!IMPORTANT]
> João Victor Cavalcante Miranda (#14582927)

## Atividade 6 — Threads

Demonstração de multithreading cooperativo/preemptivo no Zephyr OS: dois threads piscam LEDs em frequências diferentes enquanto o thread `main` imprime o uptime do sistema.

### Diagrama de conexões

| Pino (KL25Z) | Header (FRDM) | Direção | Função |
|---|---|---|---|
| PTB18 | J2-4 | Saída | LED vermelho integrado (active-low) |
| PTB19 | J2-2 | Saída | LED verde integrado (active-low) |

### Como usar

```bash
# Flash
pio run -t upload

# Monitor serial
pio device monitor --baud 115200
```

Saída esperada:
```
PSI3441 — Atividade 6: Threads
  t_red   500 ms  prio=5
  t_green 200 ms  prio=6
  main   1000 ms  prio=0
[red]   512 ms
[main]  uptime: 1001 ms
[red]   1012 ms
...
```

### Funcionamento

#### K_THREAD_DEFINE

Macro que cria um thread em tempo de compilação — pilha e estrutura de controle vão para a seção `.data` do ELF, sem necessidade de `k_thread_create()` em runtime:

```c
K_THREAD_DEFINE(nome, stack_size, função, p1, p2, p3, prioridade, opções, delay_ms);
```

Equivalente dinâmico (para comparação):

```c
K_THREAD_STACK_DEFINE(stack, STACK_SIZE);
struct k_thread tcb;
k_thread_create(&tcb, stack, STACK_SIZE, fn, NULL, NULL, NULL, 5, 0, K_NO_WAIT);
```

#### Scheduler Zephyr

O scheduler é **preemptivo por prioridade** com suporte opcional a time-slicing. Prioridades menores = maior urgência:

| Thread | Prioridade | Período | Descrição |
|---|---|---|---|
| main | 0 | 1000 ms | uptime via printk |
| t_red | 5 | 500 ms | toggle LED vermelho |
| t_green | 6 | 200 ms | toggle LED verde |

Quando um thread chama `k_msleep()`, ele é colocado em estado `suspended` e o scheduler escolhe o próximo runnable de maior prioridade. No Cortex-M0+, o PendSV realiza a troca de contexto salvando/restaurando registradores (r4–r11, PSP).

#### Por que os LEDs parecem piscar ao mesmo tempo

Cada `k_msleep(N)` insere o thread numa fila temporizada. O SysTick a 48 MHz gera interrupções periódicas que acordam os threads expirados. A resolução de scheduling é tipicamente 1 ms (`CONFIG_SYS_CLOCK_TICKS_PER_SEC=1000`).

---

## Apêndice — Conceitos relevantes

| Termo | Definição |
|---|---|
| `K_THREAD_DEFINE` | Macro que instancia thread em compile-time (sem malloc) |
| Prioridade preemptiva | Thread de maior prioridade (número menor) interrompe os demais |
| `k_msleep(ms)` | Suspende o thread por N milissegundos, liberando a CPU |
| PendSV | Exceção ARM usada pelo Zephyr para troca de contexto de baixa prioridade |
| SysTick | Timer de sistema ARM; base de tempo do Zephyr |
| `k_uptime_get_32()` | Retorna uptime em ms como `uint32_t` (suficiente para ~49 dias) |
