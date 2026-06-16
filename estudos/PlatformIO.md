# PlatformIO

Ferramenta de build, upload e gestão de dependências para sistemas embarcados. Funciona como backend do VS Code via extensão e também como CLI (`pio`).

>[!IMPORTANT]
> https://raw.githubusercontent.com/platformio/platformio-core-installer/master/get-platformio.py
>O script oficial é cara de pau, coloca lixo na minha pasta raiz. `~/.platformio`
>Não passará
### Alternativa — instalar direto como ferramenta uv
```bash
uv tool install platformio
```

Isso expõe `pio` no PATH sem precisar do script instalador. Mais limpo para quem já usa `uv` como gerenciador principal. Use `uv`.
### Verificar

```bash
pio --version
```

---

## Workflow básico

```
projeto/
  platformio.ini   ← configuração principal
  src/
    main.c         ← código da aplicação
  zephyr/
    prj.conf       ← Kconfig do Zephyr
    CMakeLists.txt ← para builds diretos com west
```

### Comandos do dia a dia

```bash
# Compilar
pio run

# Compilar + enviar para a placa (DAPLINK detectado automaticamente)
pio run -t upload

# Monitor serial
pio device monitor --baud 115200

# Limpar artefatos de build
pio run -t clean

# Listar dispositivos conectados
pio device list
```

---

## platformio.ini — anatomia

```ini
[platformio]
description = Descrição do projeto

[env:frdm_kl25z]
platform = freescalekinetis   ; família NXP Kinetis
board    = frdm_kl25z         ; placa específica
framework = zephyr            ; Zephyr OS (omitir = bare-metal)

; Flags de compilação adicionais
build_flags = -I${PROJECT_DIR}/../include

; Filtro de fontes (padrão: tudo em src/)
; src_filter = +<src/> +<lib/pwm_z42.c>
```

### Diferença entre bare-metal e Zephyr

|                       | Bare-metal            | Zephyr              |
| --------------------- | --------------------- | ------------------- |
| `framework`           | omitido               | `= zephyr`          |
| Build via PlatformIO  | sim (GCC ARM)         | sim (CMake interno) |
| `prj.conf` necessário | não                   | sim (em `zephyr/`)  |
| `startup.c` / `.ld`   | manual (Makefile.inc) | gerenciado pelo BSP |
| `printk` disponível   | não                   | sim                 |

> Para as entregas 3–5 (bare-metal Makefile), o `platformio.ini` existe apenas para IDE (IntelliSense, navegação de código). O build real continua com `make`.

---

## Integração com VS Code

Instalar a extensão **PlatformIO IDE** (`platformio.platformio-ide`).

Ao abrir uma pasta que contém `platformio.ini`, a extensão reconhece o projeto automaticamente e oferece:
- Barra de status com ações rápidas (build / upload / monitor)
- IntelliSense calibrado para o MCU alvo
- Gerenciamento de bibliotecas via `pio lib`

### Abrindo uma única entrega

```bash
code entregas/5/   # abre só a entrega 5 como projeto PlatformIO
```

### Abrindo o repositório inteiro

O arquivo `entregas/platformio.ini` (raiz das entregas) cobre o projeto Zephyr principal (`src/`). As subpastas (`3/`, `4/`, `5/`, `template/`) têm seus próprios `platformio.ini` e podem ser abertas individualmente.

---

## Upload protocol — mbed vs pyocd

O FRDM-KL25Z tem um chip DAPLINK que aparece como duas coisas no Linux:
- Um drive USB (para drag-and-drop de `.bin`)
- Um dispositivo CMSIS-DAP (para debug/flash via SWD)

### Por que `mbed` não funciona bem com Zephyr

O protocolo `mbed` (padrão do PlatformIO para essa placa) simplesmente copia o `.bin` para o drive USB. O DAPLINK decide o endereço de flash. Para bare-metal simples funciona; para Zephyr, o binário inclui kernel + app com um layout de memória específico — o DAPLINK às vezes programa no endereço errado ou não confirma se o flash foi concluído. Resultado: código não reflete na placa.

### cmsis-dap resolve

O PlatformIO para `freescalekinetis` aceita estes protocolos: `blackmagic, cmsis-dap, jlink, mbed`. O `pyocd` é instalado como ferramenta interna mas **não é um protocolo de upload válido** para esse platform — tentar usá-lo faz o PlatformIO silenciosamente cair de volta para `mbed`.

O protocolo correto é `cmsis-dap`: usa OpenOCD com transporte CMSIS-DAP (USB-HID), detecta o chip, apaga e programa setor a setor no endereço exato. É o mesmo que o log já aponta como debug tool padrão da placa (`DEBUG: Current (cmsis-dap)`).

```ini
; platformio.ini — todas as entregas (bare-metal e Zephyr)
upload_protocol = cmsis-dap
debug_tool = cmsis-dap
```

---

## Curiosidade — como o PlatformIO encontra o Zephyr SDK

Quando `framework = zephyr` é especificado, PlatformIO baixa automaticamente:
- O Zephyr SDK (toolchain ARM + headers)
- O pacote `framework-zephyr` (código-fonte do OS)
- O `platform-freescalekinetis` (suporte à placa)

O build ocorre em `.pio/build/frdm_kl25z/`. Por isso o `.gitignore` ignora `.pio/`.

O `CMakeLists.txt` em `zephyr/` é usado por `west` (ferramenta nativa Zephyr). 
O PlatformIO gera seu próprio `CMakeLists.txt` internamente — os dois coexistem sem conflito.