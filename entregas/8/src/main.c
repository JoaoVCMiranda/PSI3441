/*
 * PSI3441 — Atividade 8: Aquisição de Dados em Tempo Real
 * FRDM-KL25Z + Zephyr OS
 *
 * Arquitetura de threads:
 *   t_acquire  (prio 3) — lê ADC0_SE8 (PTB0/A0) na maior taxa possível,
 *                          aplica FIR opcional, enfileira em k_msgq
 *   t_comm     (prio 5) — retira da fila e transmite CSV pela UART
 *
 * Saída CSV  →  ts_us,raw,filtered
 * Linhas '#' são comentários (rate report via LOG_INF, configuração)
 *
 * Para medir taxa SEM filtro:
 *   build_flags = ... -DUSE_FIR=0
 * Para medir taxa COM filtro (padrão):
 *   build_flags = ... -DUSE_FIR=1
 *
 * Gargalo esperado: UART a 115200 baud ≈ 524 linhas/s → ~500 Hz max.
 * ADC de 12 bits no KL25Z atinge até ~40 kHz — muito além do canal serial.
 */

#include <zephyr.h>
#include <drivers/adc.h>
#include <sys/atomic.h>
#include <logging/log.h>

LOG_MODULE_REGISTER(ativ8, LOG_LEVEL_INF);

/* ── ADC ───────────────────────────────────────────────────────────────── */
#define ADC_NODE        DT_NODELABEL(adc0)
#define ADC_CHANNEL_ID  8     /* ADC0_SE8 = PTB0 (header A0) */
#define ADC_RESOLUTION  12    /* 0 … 4095 */

static int16_t adc_raw_buf;

static const struct adc_channel_cfg ch_cfg = {
    .gain             = ADC_GAIN_1,
    .reference        = ADC_REF_VDD_1,     /* ref = VDD = 3,3 V */
    .acquisition_time = ADC_ACQ_TIME_DEFAULT,
    .channel_id       = ADC_CHANNEL_ID,
    .differential     = 0,                  /* single-ended */
};

static struct adc_sequence seq = {
    .channels    = BIT(ADC_CHANNEL_ID),
    .buffer      = &adc_raw_buf,
    .buffer_size = sizeof(adc_raw_buf),
    .resolution  = ADC_RESOLUTION,
};

/* ── FIR 8-tap — média móvel em ponto fixo ─────────────────────────────
 * Coeficientes: h[k] = 1/8 para k = 0..7  (todos iguais → caixa retangular)
 * Implementação: buffer circular de tamanho 2^N; shift 3 = divisão por 8.
 * Sem ponto flutuante → zero overhead de softfloat no Cortex-M0+.
 * Resposta em frequência: zeros em fs/8, fs/4, 3fs/8, fs/2 (rejeita ruído HF).
 *
 * Para desabilitar em tempo de build: -DUSE_FIR=0 no platformio.ini.      */
#ifndef USE_FIR
#define USE_FIR 1
#endif

#define FIR_TAPS 8   /* deve ser potência de 2 */

static int16_t fir_history[FIR_TAPS]; /* inicializado com 0 (BSS) */
static uint8_t fir_head;

static inline int16_t fir_apply(int16_t sample)
{
    fir_history[fir_head++ & (FIR_TAPS - 1)] = sample;
    int32_t sum = 0;
    for (int i = 0; i < FIR_TAPS; i++) {
        sum += fir_history[i];
    }
    return (int16_t)(sum >> 3); /* /8 */
}

/* ── Mensagem entre threads ────────────────────────────────────────────── */
struct sample {
    uint32_t ts_us;    /* timestamp em µs desde o boot */
    int16_t  raw;
    int16_t  filtered;
};

/* 64 slots: se t_comm não esvaziar a tempo, t_acquire descarta a amostra
 * e incrementa dropped_count em vez de bloquear a aquisição.             */
K_MSGQ_DEFINE(sample_q, sizeof(struct sample), 64, 4);

static atomic_t dropped_count;

/* ── Timestamp em µs ───────────────────────────────────────────────────── */
/* KL25Z a 48 MHz → 48 ciclos por µs */
#define CPU_MHZ 48U

static inline uint32_t cycles_to_us(uint32_t cyc)
{
    return cyc / CPU_MHZ;
}

/* ── Thread de aquisição ───────────────────────────────────────────────── */
#define RATE_WINDOW  200   /* amostras por janela de medição */
#define STACK_SIZE  1024

static void fn_acquire(void *p1, void *p2, void *p3)
{
    const struct device *adc = DEVICE_DT_GET(ADC_NODE);
    if (!device_is_ready(adc)) {
        LOG_ERR("ADC não está pronto");
        return;
    }
    adc_channel_setup(adc, &ch_cfg);
    LOG_INF("ADC pronto — canal %d, %d bits", ADC_CHANNEL_ID, ADC_RESOLUTION);

    uint32_t window_start = k_cycle_get_32();
    uint32_t count = 0;

    for (;;) {
        uint32_t t_cyc = k_cycle_get_32();
        adc_read(adc, &seq);

        int16_t raw      = adc_raw_buf;
        int16_t filtered = USE_FIR ? fir_apply(raw) : raw;

        struct sample s = {
            .ts_us    = cycles_to_us(t_cyc),
            .raw      = raw,
            .filtered = filtered,
        };

        /* K_NO_WAIT: nunca bloqueia — descarta se a fila estiver cheia */
        if (k_msgq_put(&sample_q, &s, K_NO_WAIT) != 0) {
            atomic_inc(&dropped_count);
        }

        if (++count % RATE_WINDOW == 0) {
            uint32_t dt_us = cycles_to_us(k_cycle_get_32() - window_start);
            uint32_t rate  = (uint32_t)((uint64_t)RATE_WINDOW * 1000000U / dt_us);
            uint32_t drop  = (uint32_t)atomic_get(&dropped_count);
            atomic_set(&dropped_count, 0);

            /* LOG_INF é assíncrono (deferred): vai para o ring buffer do LOG.
             * Se o processamento estiver saturado, mensagens são descartadas.
             * Isso é visível no monitor como linhas '#' ausentes.            */
            LOG_INF("taxa: %u Hz  descartadas: %u  fila: %u/%u  FIR=%s",
                    rate, drop,
                    k_msgq_num_used_get(&sample_q),
                    k_msgq_num_free_get(&sample_q) + k_msgq_num_used_get(&sample_q),
                    USE_FIR ? "on" : "off");
            window_start = k_cycle_get_32();
        }
    }
}

/* ── Thread de comunicação ─────────────────────────────────────────────── */
static void fn_comm(void *p1, void *p2, void *p3)
{
    struct sample s;

    /* cabeçalho para o script Python identificar o modo */
    printk("# PSI3441 Atividade 8  FIR=%s\n", USE_FIR ? "on" : "off");
    printk("# ts_us,raw,filtered\n");
    printk("# UART 115200 baud  max teórico: ~524 Hz (22 bytes/linha)\n");

    for (;;) {
        /* bloqueia até haver amostra — CPU fica livre para t_acquire */
        k_msgq_get(&sample_q, &s, K_FOREVER);

        /* printk é síncrono: não perde bytes mas bloqueia enquanto UART transmite.
         * A cada linha de ~22 bytes @ 115200 baud: ~1,9 ms de bloqueio.
         * Isso limita t_comm a ~524 Hz — e t_acquire vai descartar o excesso. */
        printk("%u,%d,%d\n", s.ts_us, (int)s.raw, (int)s.filtered);
    }
}

K_THREAD_DEFINE(t_acq,  STACK_SIZE, fn_acquire, NULL, NULL, NULL, 3, 0, 0);
K_THREAD_DEFINE(t_comm, STACK_SIZE, fn_comm,    NULL, NULL, NULL, 5, 0, 0);

void main(void)
{
    LOG_INF("PSI3441 — Atividade 8 iniciada");
    k_sleep(K_FOREVER);
}
