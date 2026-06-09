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
