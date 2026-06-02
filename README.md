# PSI3441

## Arquitetura de Sistemas Embarcados - Repositório de Entregas

> Fiquei muito satisfeito de enfim poder usar uma ferramenta de gestão de versão útil para aplicação no dia a dia nas tarefas da faculdade.

Para cada pasta tem o número da atividade que foi entregue.

---

## Roteiro de Compilação

### Pré-requisitos

| Ferramenta | Versão mínima | Instalação (Arch) |
|---|---|---|
| `arm-none-eabi-gcc` | 12+ | `sudo pacman -S arm-none-eabi-gcc` |
| `arm-none-eabi-binutils` | — | `sudo pacman -S arm-none-eabi-binutils` |
| `make` | — | já incluso |

Verificar instalação:

```bash
arm-none-eabi-gcc --version
```

### Build de uma atividade

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

| Arquivo | Uso |
|---|---|
| `output.elf` | debug com GDB/OpenOCD |
| `output.bin` | gravação direta via DAPLINK |
| `output.map` | mapa de símbolos e seções |

### Gravação na placa (DAPLINK)

A FRDM-KL25Z expõe um chip DAPLINK via USB que aparece como pendrive:

1. Conectar a placa ao computador via cabo USB (conector SDA, não o KL25Z).
2. O sistema monta um volume chamado `FRDM-KL25Z` (ou `DAPLINK`).
3. Copiar o binário gerado:
   ```bash
   cp output.bin /run/media/$USER/FRDM-KL25Z/
   ```
4. O LED vermelho da placa pisca durante a gravação.
5. A placa reseta automaticamente e executa o novo firmware.

> Se o volume montar como `MAINTENANCE`, significa que o firmware do DAPLINK está desatualizado. Gravar o `.bin` do DAPLINK atualizado resolve.

### Depuração com GDB (opcional)

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

---

## Gravação via Android (Termux + OpenOCD)

A FRDM-KL25Z expõe uma interface CMSIS-DAP via DAPLINK. Com um cabo USB-C OTG e o OpenOCD rodando no Termux é possível gravar e depurar sem computador.

### Pré-requisitos no Android

- Android 7+ com suporte USB Host (OTG)
- [Termux](https://f-droid.org/packages/com.termux/) instalado via F-Droid (não a versão da Play Store — está desatualizada)
- [Termux:API](https://f-droid.org/packages/com.termux.api/) instalado (necessário para `termux-usb`)
- Cabo USB-C OTG + adaptador USB-A para o conector SDA da placa

### 1. Instalar pacotes no Termux

```bash
pkg update && pkg upgrade
pkg install termux-api openocd
```

### 2. Transferir o binário para o Android

Qualquer método serve — ADB, syncthing, cloud drive:

```bash
# exemplo via ADB a partir do notebook
adb push entregas/5/output.bin /sdcard/Download/output.bin
```

No Termux, mover para um caminho acessível:

```bash
cp /sdcard/Download/output.bin ~/output.bin
```

### 3. Conceder acesso USB ao Termux (sem root)

O `termux-usb` envolve a USB API do Android para passar o file descriptor ao processo filho.
Conecte a placa e execute:

```bash
# lista os dispositivos USB reconhecidos pelo Android
termux-usb -l
# saída esperada: ["/dev/bus/usb/001/002"]  ← caminho do DAPLINK

# abre o device e executa o OpenOCD com acesso ao fd
termux-usb -e "openocd -f interface/cmsis-dap.cfg \
  -f target/klx.cfg \
  -c 'program /root/output.bin verify reset exit 0x00000000'" \
  /dev/bus/usb/001/002
```

> Substitua `/dev/bus/usb/001/002` pelo caminho retornado em `termux-usb -l`.
> Na primeira execução o Android exibe um diálogo pedindo permissão de acesso ao dispositivo USB — confirmar "Permitir sempre".

### 4. Verificar gravação

O OpenOCD imprime a sequência abaixo em caso de sucesso:

```
** Programming Started **
** Programming Finished **
** Verify Started **
** Verified OK **
** Resetting Target **
shutdown command invoked
```

A placa reseta automaticamente e o firmware entra em execução.

### Alternativa: massa de armazenamento (sem OpenOCD)

Se o Android montar o volume DAPLINK automaticamente, qualquer gerenciador de arquivos com suporte OTG (Solid Explorer, Total Commander + USB plugin) resolve:

1. Abrir o gerenciador → navegar até o volume `FRDM-KL25Z`
2. Copiar `output.bin` para a raiz do volume
3. Aguardar LED piscar e placa resetar

### Troubleshooting

| Sintoma | Causa provável | Solução |
|---|---|---|
| `termux-usb -l` retorna `[]` | Android não reconheceu o DAPLINK | Desconectar e reconectar; testar outro cabo |
| `Error: unable to open CMSIS-DAP device` | Permissão negada ou conflito com MSD | Revogar acesso de outros apps ao dispositivo USB nas configurações do Android |
| Volume monta como `MAINTENANCE` | Firmware DAPLINK desatualizado | Gravar firmware DAPLINK atualizado (`.bin` no site da NXP) via computador primeiro |
| OpenOCD trava em `Waiting for target...` | KL25Z não está em modo de debug | Pressionar RESET na placa enquanto o OpenOCD aguarda |

