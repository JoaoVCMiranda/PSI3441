TeleTYpe
Aparente a história começa em 1800 com teleimpressoras remotas.

É a "porta" que dispositivos de hardware se comunicam.

```
$ tty
/dev/pts/1
```

Essa pasta `dev/pts` parece importante (especialmente se já fez uma instalação com chroot) já que ela mapeia algumas coisa! gostaria de aprender mais sobre!

# Permissões
No linux, para acessar algumas portas seriais (ex: `/dev/ttyACM0`) como usuário comum, é preciso adicionar o usuário comum aos grupos

```bash 
ls -l /dev/ttyACM0
```

```plaintext
crw-rw---- 166,0 root 16 jun 09:32 󰡯 /dev/ttyACM0
```

Veja que o acesso a porta `/dev/ttyACM0` está no grupo `root` apenas.

Para resolver você pode fazer um grupo, ou mudar o dono do arquivo diretamente com 

```sh 
sudo chwon <user>:<user> /dev/tty/ACM0
```

> Deve ser feito ao iniciar o monitoramento após reiniciar o computador ou adicionar o usuário ao grupo de leitura de `/dev/ttyACM0`