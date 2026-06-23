> [!IMPORTANT]
> João Victor Cavalcante Miranda (#14582927)
> https://github.com/JoaoVCMiranda/PSI3441/blob/96cc05d64c7d144f835fdc9bdb354a3d1ddfa3c7/entregas/7/src/main.c

## Atividade 7 — Shared Resources

Um producer incrementa um contador a cada 100 ms e um consumer lê, imprime e zera a cada 500 ms. O acesso ao contador é protegido por `k_mutex`.

Sem o mutex, a sequência leitura → modificação → escrita pode ser interrompida no meio pelo scheduler — o consumer poderia ler um valor inconsistente ou o producer sobrescrever um zero feito pelo consumer. No Cortex-M0+ isso é menos óbvio que em sistemas multicore, mas o problema existe: o compilador pode manter a variável em registrador entre operações, e uma preempção no meio causa exatamente o race condition descrito.

O `k_msleep(500)` do consumer fica fora da seção crítica intencionalmente — segurar o mutex durante um sleep bloquearia o producer inteiro por 500 ms. Seção crítica deve ser mínima.
