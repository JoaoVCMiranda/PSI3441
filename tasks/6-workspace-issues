Eu gostaria de executar o código de cada entrega sincronizadamente no mesmo repositório se for necessário


Para isso fiz o repositório principal com vários projetos do platformIO e um arquivo de workspace em .vscode/

Porém, o platformIO obriga a criação da pasta ~/.platformIO salve na memória que eu detesto que coloquem lixo na minha pasta raiz, então vou procurar uma forma melhor de usar.

mesmo após algumas alterações continuo com o seguinte ao fazer build e upload para placa


```plaintext
rocessing frdm_kl25z (platform: freescalekinetis; board: frdm_kl25z)
----------------------------------------------------------------------------------------------------------------------------------------------------------------------------
Verbose mode can be enabled via `-v, --verbose` option
CONFIGURATION: https://docs.platformio.org/page/boards/freescalekinetis/frdm_kl25z.html
PLATFORM: Freescale Kinetis (10.0.0) > Freescale Kinetis FRDM-KL25Z
HARDWARE: MKL25Z128VLK4 48MHz, 16KB RAM, 128KB Flash
DEBUG: Current (cmsis-dap) On-board (cmsis-dap) External (blackmagic, jlink)
PACKAGES: 
 - tool-pyocd @ 1.2900.210122 (29.0) 
 - toolchain-gccarmnoneeabi @ 1.80201.181220 (8.2.1)
LDF: Library Dependency Finder -> https://bit.ly/configure-pio-ldf
LDF Modes: Finder ~ chain, Compatibility ~ soft
Found 0 compatible libraries
Scanning dependencies...
No dependencies
Building in release mode
Checking size .pio/build/frdm_kl25z/firmware.elf
Advanced Memory Usage is available via "PlatformIO Home > Project Inspect"
RAM:   [          ]   0.8% (used 124 bytes from 16384 bytes)
Flash: [          ]   0.4% (used 500 bytes from 131072 bytes)
```
Até aqui está ok. O problema está no upload, que não reflete o que está sendo compilado e usa o protocolo mbed, já instalei o pyocd com uv.

```plaintext
Configuring upload protocol...
AVAILABLE: blackmagic, cmsis-dap, jlink, mbed
CURRENT: upload_protocol = mbed
Looking for upload disk...
Auto-detected: /media/jvcm/DAPLINK
Uploading .pio/build/frdm_kl25z/firmware.bin
Firmware has been successfully uploaded.
(Some boards may require manual hard reset)
======================================================================= [SUCCESS] Took 0.66 seconds =======================================================================
 *  Terminal will be reused by tasks, press any key to close it
```
