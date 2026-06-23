> [!IMPORTANT]
> João Victor Cavalcante Miranda (#14582927)
> https://github.com/JoaoVCMiranda/PSI3441/blob/6e6f7f305bd9b252c8c748fcb1b8420e154f04d8/entregas/5/src/main.c

## Atividade 5 — HC-SR04 + PWM radar (Zephyr híbrido)

Bem, parece que usar a API do Zephyr deixar muita coisa mais simples. Já que ela absorve a complexidade de lidar com as portas e registradores individualmente e em baixo nível.

Essencialmente tudo foi reescrito para utilizá-la.

Porém isso simplificou bastante a carga cognitiva do código.

O acesso ao TPM2->MOD continua "bare-metal" pois é o conceito da aula compreender o funcionamento dos timers. E como eles se relacionam com as portas da placa oferece.