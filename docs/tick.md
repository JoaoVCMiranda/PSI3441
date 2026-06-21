Para testar se o `printk` está funcionando usei

```c
#include <zephyr.h>
#include <drivers/gpio.h>

void main(void) {
    printk("boot ok\n");
    while (1) {
        printk("tick\n");
        k_msleep(1000);
    }
}
```