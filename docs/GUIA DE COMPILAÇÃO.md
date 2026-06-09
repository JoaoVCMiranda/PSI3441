# Pré-requisitos

| Ferramenta                | Instalação (Arch)                       |
| ------------------------- | --------------------------------------- |
| `arm-none-eabi-gcc` (12+) | `sudo pacman -S arm-none-eabi-gcc`      |
| `arm-none-eabi-binutils`  | `sudo pacman -S arm-none-eabi-binutils` |
| `make`                    | já incluso                              |
|                           |                                         |

Verificar instalação:

```bash
arm-none-eabi-gcc --version
```

## Build de uma atividade

```bash
cd entregas/<N>   # ex: cd entregas/5
make              # gera output.elf, output.bin e output.map
make clean        # remove artefatos gerados
```

O `Makefile` de cada entrega inclui `utils/Makefile.inc`, que:
- compila para Cortex-M0+ (`-mcpu=cortex-m0plus -mthumb`)
- linka com `utils/kl25z.ld` (flash 128 KB @ 0x0, RAM 16 KB @ 0x1FFFF000)
- inclui `utils/startup.c` (tabela de vetores + campo de configuração de flash obrigatório no KL25Z)

Saídas:

| Arquivo      | Uso                         |
| ------------ | --------------------------- |
| `output.elf` | debug com GDB/OpenOCD       |
| `output.bin` | gravação direta via DAPLINK |
| `output.map` | mapa de símbolos e seções   |

## Gravação na placa (DAPLINK)

A FRDM-KL25Z expõe um chip DAPLINK via USB que aparece como pendrive:

1. Conectar a placa ao computador via cabo USB (conector SDA, não o KL25Z).
2. O sistema monta um volume chamado `FRDM-KL25Z` (ou `DAPLINK`).
3. Copiar o binário gerado:
   ```bash
   cp output.bin /run/media/$USER/FRDM-KL25Z/
   ```
4. O LED RX/TX da placa pisca durante a gravação.
5. A placa reseta automaticamente e executa o novo firmware.

> Se o volume montar como `MAINTENANCE`, significa que o firmware do DAPLINK está desatualizado. Gravar o `.bin` do DAPLINK atualizado resolve.

## Depuração com GDB (opcional)

```bash
# terminal 1 — servidor OpenOCD
openocd -f interface/cmsis-dap.cfg -f target/klx.cfg

# terminal 2 — cliente GDB
arm-none-eabi-gdb output.elf
(gdb) target extended-remote :3333
(gdb) monitor reset halt
(gdb) load
(gdb) continue
```
