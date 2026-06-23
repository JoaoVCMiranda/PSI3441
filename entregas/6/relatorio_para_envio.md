> [!IMPORTANT]
> João Victor Cavalcante Miranda (#14582927)
> https://github.com/JoaoVCMiranda/PSI3441/blob/fbe7c41961f7d302a3c9cc7c3706a416f09b588f/entregas/6/src/main.c

## Atividade 6 — Threads

Dois threads piscam LEDs em frequências diferentes enquanto o `main` imprime o uptime — tudo rodando aparentemente em paralelo num único núcleo Cortex-M0+.

O `K_THREAD_DEFINE` cria os threads em tempo de compilação, sem alocar memória em runtime. O scheduler é preemptivo por prioridade: quando um thread chama `k_msleep`, ele é suspenso e o próximo de maior prioridade assume. A troca de contexto no ARM é feita via PendSV, que salva e restaura os registradores da CPU.

O que ficou claro nessa entrega é que "paralelo" aqui não é paralelismo real — é intercalação controlada pelo scheduler. A ilusão funciona porque os períodos são diferentes e as trocas são rápidas o suficiente para o olho não perceber.
