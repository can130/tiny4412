obj-m := tiny4412_leds.o  
  
KDIR := /home/workplace/Tiny4412/linux-3.0.86/   # arm  
  
all:  
    make -C $(KDIR) M=$(PWD) modules ARCH=arm CROSS_COMPILE=arm-linux-   
    arm-linux-gcc -static led_oper.c -o led_oper  
      
clean:  
    rm -f *.ko *.o *.mod.o *.mod.c *.symvers modul* *oper  