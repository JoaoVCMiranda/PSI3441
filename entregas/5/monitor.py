#!/usr/bin/env python3
# /// script
# dependencies = ["pyserial"]
# ///
"""
Le a saida serial do radar HC-SR04 (PSI3441 5-1) e mostra a distancia
em tempo real no terminal.

Uso:
    uv run distance_monitor.py [porta] [baud]

Exemplo:
    uv run distance_monitor.py /dev/ttyACM0 115200
"""
import re
import shutil
import sys

import serial

PORT = sys.argv[1] if len(sys.argv) > 1 else "/dev/ttyACM0"
BAUD = int(sys.argv[2]) if len(sys.argv) > 2 else 115200

# Casa com a linha que o firmware imprime:
#   echo=  123 us  dist= 12 cm  mod= 6789
PATTERN = re.compile(r"dist=\s*(\d+)\s*cm")

BAR_SCALE = 2  # cm por caractere da barra
CLEAR_LINE = "\x1b[2K"  # apaga a linha inteira do terminal (evita sobra de caracteres)


def main() -> None:
    with serial.Serial(PORT, BAUD, timeout=1) as ser:
        print(f"Lendo {PORT} @ {BAUD} baud — Ctrl+C para sair\n")
        while True:
            line = ser.readline().decode(errors="ignore").strip()
            if not line:
                continue

            match = PATTERN.search(line)
            if not match:
                continue

            dist = int(match.group(1))
            # escala a barra pra largura atual do terminal, evitando quebra de linha
            term_width = shutil.get_terminal_size((80, 20)).columns
            bar_max = max(term_width - 14, 10)
            bar = "#" * min(dist // BAR_SCALE, bar_max)
            sys.stdout.write(f"{CLEAR_LINE}\r{dist:4d} cm  {bar}")
            sys.stdout.flush()


if __name__ == "__main__":
    try:
        main()
    except KeyboardInterrupt:
        print("\nEncerrado.")
    except serial.SerialException as exc:
        print(f"\nErro abrindo a porta serial: {exc}")