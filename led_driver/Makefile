obj-m := led.o  
  
KDIR :=/home/can/tiny4412/linux-3.8  # arm  
all:  
	make -C $(KDIR) M=$(PWD) modules ARCH=arm CROSS_COMPILE=arm-linux-   
	arm-linux-gcc -static led_test.c -o led_test
      
clean:  
	rm -f *.ko *.o *.mod.o *.mod.c *.symvers modul* *oper  
