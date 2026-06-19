# Instalação

> Das bibliotecas e afins

```sh
./install.sh
```

# VS Code

Adicione o `Embarcados.code-profile` 
- [ ] PlatformIO no VS Code
# [The Zephyr OS ](https://docs.zephyrproject.org/latest/introduction/index.html)

É o Real-Time Operating System(RTOS) da matéria 
## [Getting started](https://docs.zephyrproject.org/latest/develop/getting_started/index.html)

## PlatformIO + VSCode via uv

```bash
uv tool install platformio \
  --with pip \
  --with pyocd \
  --python 3.11
```

- `--with pip` — necessário para o PlatformIO instalar dependências de uploaders internamente  
- `--with pyocd` — uploader do FRDM-KL25Z (CMSIS-DAP)  
- `--python 3.11` — Zephyr 2.7.x usa `distutils`, removido no Python 3.12  

### PATH (fish)

```fish
fish_add_path ~/.local/share/uv/tools/platformio/bin
```

### Atualizar

```bash
uv tool upgrade platformio
```

### Configuração do workspace (`.code-workspace`)

```json
{
  "folders": [
    { "path": "." }
  ],
  "settings": {
    "platformio-ide.useBuiltinPIOCore": false,
    "platformio-ide.useBuiltinPython": false,
    "platformio-ide.customPATH": "/home/USER/.local/share/uv/tools/platformio/bin"
  }
}
```

> Substituir `USER` pelo seu usuário. As settings devem estar no `.code-workspace`  
> e não no user/profile settings do VSCode para serem respeitadas pela extensão.