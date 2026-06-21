#!/usr/bin/env python3
"""
PSI3441 — Atividade 8 — Visualização em tempo real
Lê CSV de /dev/ttyACM0 e plota raw vs FIR em tempo real.

Dependências:
    pip install pyserial matplotlib

Uso:
    python monitor.py [porta] [baud]

Exemplos:
    python monitor.py /dev/ttyACM0 115200
    python monitor.py COM3 115200
"""

import sys
import threading
import time
from collections import deque

import serial
import matplotlib.pyplot as plt
import matplotlib.animation as animation

# ── Configuração ──────────────────────────────────────────────────────────
PORT   = sys.argv[1] if len(sys.argv) > 1 else '/dev/ttyACM0'
BAUD   = int(sys.argv[2]) if len(sys.argv) > 2 else 115200
WINDOW = 800   # amostras exibidas simultaneamente

# ── Buffers compartilhados (thread serial + thread matplotlib) ────────────
lock    = threading.Lock()
t_ms    = deque(maxlen=WINDOW)
d_raw   = deque(maxlen=WINDOW)
d_fir   = deque(maxlen=WINDOW)

state = {
    'total'   : 0,
    'rate_hz' : 0.0,
    'fir'     : '?',
    't0_us'   : None,
}

# ── Thread de leitura serial ──────────────────────────────────────────────
def read_serial(ser: serial.Serial) -> None:
    count = 0
    t_rate = time.monotonic()

    while True:
        try:
            line = ser.readline().decode('ascii', errors='ignore').strip()
        except serial.SerialException:
            break

        if not line:
            continue

        if line.startswith('#'):
            # extrai modo FIR do cabeçalho
            if 'FIR=' in line:
                with lock:
                    state['fir'] = 'on' if 'FIR=on' in line else 'off'
            continue

        parts = line.split(',')
        if len(parts) != 3:
            continue

        try:
            ts_us = int(parts[0])
            raw   = int(parts[1])
            fir   = int(parts[2])
        except ValueError:
            continue

        with lock:
            if state['t0_us'] is None:
                state['t0_us'] = ts_us
            t_ms.append((ts_us - state['t0_us']) / 1000.0)
            d_raw.append(raw)
            d_fir.append(fir)
            state['total'] += 1
            count += 1

        now = time.monotonic()
        if now - t_rate >= 1.0:
            with lock:
                state['rate_hz'] = count / (now - t_rate)
            count = 0
            t_rate = now

# ── Figura ────────────────────────────────────────────────────────────────
fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(13, 7), sharex=True)
fig.suptitle('PSI3441 — Atividade 8: Aquisição ADC em Tempo Real', fontsize=13)

(ln_raw,) = ax1.plot([], [], color='steelblue', lw=0.8, label='Raw ADC')
(ln_fir,) = ax2.plot([], [], color='tomato',    lw=0.8, label='FIR filtrado')

for ax in (ax1, ax2):
    ax.set_ylabel('ADC (12 bits)')
    ax.set_ylim(-50, 4145)
    ax.legend(loc='upper right')
    ax.grid(alpha=0.3)

ax2.set_xlabel('Tempo (ms)')

info = ax1.text(
    0.01, 0.84, '', transform=ax1.transAxes,
    fontsize=9, family='monospace',
    bbox=dict(boxstyle='round', facecolor='wheat', alpha=0.6),
)

def animate(frame):
    with lock:
        if not t_ms:
            return ln_raw, ln_fir, info
        ts   = list(t_ms)
        raws = list(d_raw)
        firs = list(d_fir)
        s    = dict(state)

    ln_raw.set_data(ts, raws)
    ln_fir.set_data(ts, firs)

    xmin = ts[0]
    xmax = max(ts[-1], xmin + 500)
    ax1.set_xlim(xmin, xmax)

    # calcula taxa efetiva a partir dos timestamps (mais preciso que o contador)
    if len(ts) >= 2:
        dt_s = (ts[-1] - ts[0]) / 1000.0
        ts_rate = len(ts) / dt_s if dt_s > 0 else 0.0
    else:
        ts_rate = 0.0

    info.set_text(
        f"Taxa efetiva : {ts_rate:6.0f} Hz\n"
        f"Taxa Python  : {s['rate_hz']:6.0f} Hz\n"
        f"Amostras     : {s['total']:>8d}\n"
        f"FIR          : {s['fir']}"
    )
    return ln_raw, ln_fir, info

def main():
    try:
        ser = serial.Serial(PORT, BAUD, timeout=0.5)
    except serial.SerialException as exc:
        print(f"Erro ao abrir {PORT}: {exc}")
        sys.exit(1)

    print(f"Conectado: {PORT} @ {BAUD} baud")
    print("Ctrl+C ou fechar a janela para encerrar.")

    reader = threading.Thread(target=read_serial, args=(ser,), daemon=True)
    reader.start()

    ani = animation.FuncAnimation(fig, animate, interval=50, blit=True)
    plt.tight_layout()

    try:
        plt.show()
    except KeyboardInterrupt:
        pass

    ser.close()

if __name__ == '__main__':
    main()
