> [!IMPORTANT]
> João Victor Cavalcante Miranda (#14582927)

## Atividade 7 — Shared Resources

Dois threads compartilham um contador inteiro. O acesso é protegido por `k_mutex` para garantir consistência. O producer incrementa a cada 100 ms; o consumer lê, imprime e zera a cada 500 ms — esperado: ~5 eventos por leitura.

### Diagrama de conexões

| Pino (KL25Z) | Header (FRDM) | Direção | Função |
|---|---|---|---|
| PTB18 | J2-4 | Saída | LED vermelho — pulsa com o producer |
| PTB19 | J2-2 | Saída | LED verde — pulsa com o consumer |

### Como usar

```bash
# Flash
pio run -t upload

# Monitor serial
pio device monitor --baud 115200
```

Saída esperada:
```
PSI3441 — Atividade 7: Shared Resources
  producer: +1 a cada 100 ms
  consumer: snapshot+reset a cada 500 ms
eventos/500ms: 5
eventos/500ms: 5
eventos/500ms: 4   ← jitter de scheduling
...
```

### Funcionamento

#### Por que mutex é necessário

A sequência **read → modify → write** de `shared_count` não é atômica:

```
Producer:  LOAD r0, [shared_count]   ; lê 4
           ADD  r0, r0, #1            ; calcula 5
           -- preempção aqui! --
Consumer:  LOAD r1, [shared_count]   ; também lê 4  ← valor antigo!
           STORE r1, [snap]
           STORE #0, [shared_count]  ; zera para 0
-- producer retoma --
           STORE r0, [shared_count]  ; escreve 5  ← perdeu o reset!
```

Com o mutex, apenas um thread entra na seção crítica por vez.

#### k_mutex vs. k_sem

| | `k_mutex` | `k_sem` |
|---|---|---|
| Tem "dono" | sim — só quem travou pode destrancar | não — qualquer um pode dar `give()` |
| Pode usar em ISR | não (`K_FOREVER` não é válido em ISR) | sim — `k_sem_give()` é ISR-safe |
| Priority inheritance | sim (Zephyr implementa) | não aplicável |
| Uso típico | exclusão mútua (seção crítica) | sinalização de eventos |

#### Seção crítica mínima

```c
k_mutex_lock(&count_mutex, K_FOREVER);
snap = shared_count;
shared_count = 0;
k_mutex_unlock(&count_mutex);
```

O `k_msleep(500)` fica **fora** da seção crítica: segurar o mutex durante um sleep bloquearia o producer por 500 ms — violação do princípio de seção crítica curta.

#### Priority inversion

Se um thread de baixa prioridade segura o mutex e um de alta prioridade tenta adquiri-lo, o de alta bloqueia. O Zephyr resolve isso com **priority inheritance**: o thread que segura o mutex temporariamente herda a prioridade do waiter mais urgente.

---

## Apêndice — Conceitos relevantes

| Termo | Definição |
|---|---|
| `K_MUTEX_DEFINE` | Declara e inicializa mutex em compile-time |
| `k_mutex_lock(m, timeout)` | Adquire mutex; `K_FOREVER` bloqueia indefinidamente |
| Race condition | Resultado depende da ordem de execução de threads concorrentes |
| Priority inheritance | Técnica para evitar inversão de prioridade em mutexes |
| Seção crítica | Trecho de código que acessa recurso compartilhado; deve ser mínimo |
| Atomic RMW | Read-Modify-Write atômico; no Cortex-M0+: via LDREX/STREX ou mutex |
