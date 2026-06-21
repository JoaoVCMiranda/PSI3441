> [!IMPORTANT]
> João Victor Cavalcante Miranda (#14582927)

## Atividade 7 — Shared Resources

Dois threads compartilham um contador inteiro. O acesso é protegido por `k_mutex` para garantir consistência. O producer incrementa a cada 100 ms; o consumer lê, imprime e zera a cada 500 ms — esperado: ~5 eventos por leitura.

### Diagrama de conexões

| Pino (KL25Z) | Header (FRDM) | Direção | Função |
|---|---|---|---|
| PTB18 | J2-4 | Saída | LED vermelho — pulsa com o producer |
| PTB19 | J2-2 | Saída | LED verde — pulsa com o consumer |

### Roteiro de configuração e execução

1. Copiar a entrega 6 como base (`cp -r entregas/6 entregas/7`) — mesmos pinos de LED
2. Editar `zephyr/CMakeLists.txt` — renomear projeto para `PSI3441_7`
3. `zephyr/prj.conf` — sem mudanças (GPIO + console são suficientes; mutex é parte do kernel)
4. Reescrever `src/main.c`: adicionar `K_MUTEX_DEFINE`, `shared_count`, lógica producer/consumer
5. Adicionar `"path": "../entregas/7"` ao workspace `.vscode/PSI3441.code-workspace`
6. Build e flash:

```bash
pio run -t upload
```

7. Monitor serial:

```bash
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

#### Conexões com outras entregas

| Entrega | O que foi reutilizado / o que mudou |
|---|---|
| 4 | ADC como fonte de dado analógico — aqui o "dado" é simulado por um contador, mas o padrão read→process→output é o mesmo |
| 5 | Na atividade 5 havia um único loop medindo e controlando; aqui a medição (producer) e o report (consumer) são threads separados — a mesma separação de responsabilidades, agora com proteção formal |
| 6 | Reutiliza `K_THREAD_DEFINE` e os mesmos LEDs; adiciona a primitiva de sincronização `k_mutex` sobre a estrutura de threads da entrega anterior |

O padrão **producer/consumer com mutex** é a base de qualquer pipeline de dados em RTOS — a entrega 8 evolui isso para `k_msgq` que elimina o acesso compartilhado completamente.

#### Sugestões de melhoria

- Adicionar um segundo contador `uint64_t total_events` (sem reset) protegido pelo mesmo mutex, para mostrar que múltiplos campos podem ser protegidos por uma única seção crítica.
- Experimentar retirar o mutex e aumentar a frequência do producer para tornar o race condition visível no serial (valores inconsistentes de `snap`).
- Substituir `uint32_t shared_count` por uma struct com timestamp e valor — mostra que mutex protege estruturas complexas da mesma forma que escalares.

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
