#include <stdio.h>     
#include <sys/types.h>    
#include <sys/stat.h>  
#include <fcntl.h>  
#include <linux/ioctl.h>  
  
  
#define DEVICE_NAME     "/dev/tiny4412-leds"  
  
//幻数  
#define LEDS_MAGIC  'a'  
#define LEDS_PGM4_0_ON      _IO(LEDS_MAGIC, 1)  
#define LEDS_PGM4_0_OFF _IO(LEDS_MAGIC, 2)  
#define LEDS_PGM4_1_ON      _IO(LEDS_MAGIC, 3)  
#define LEDS_PGM4_1_OFF _IO(LEDS_MAGIC, 4)  
#define LEDS_PGM4_2_ON      _IO(LEDS_MAGIC, 5)  
#define LEDS_PGM4_2_OFF _IO(LEDS_MAGIC, 6)  
#define LEDS_PGM4_3_ON      _IO(LEDS_MAGIC, 7)  
#define LEDS_PGM4_3_OFF _IO(LEDS_MAGIC, 8)  
  
  
int main (int argc, char *argv[])   
{  
    int leds_fd = 0;  
    int cmd;  
      
    if  (argc < 2) {  
        printf("please entry the correct operation parameter! \n");  
        return 0;  
    }  
  
    leds_fd = open(DEVICE_NAME, O_RDWR);  
    if (leds_fd == -1) {  
        printf("open device faild! \n");  
        return 0;  
    }   
  
    cmd = atoi(argv[1]);        //把终端上收到的字符串命令转换成整型  
    //printf("argv:%s \n", argv[1]);  
        /* 执行 ./led_oper 11 点亮LED1 ， 执行./led_oper 10 关闭LED1，其他LED灯以此类推 */  
    switch (cmd) {  
    case 10:  ioctl(leds_fd, LEDS_PGM4_0_OFF);  
        break;  
    case 11:  ioctl(leds_fd, LEDS_PGM4_0_ON);  
        break;  
    case 20:  ioctl(leds_fd, LEDS_PGM4_1_OFF);  
        break;  
    case 21:  ioctl(leds_fd, LEDS_PGM4_1_ON);  
        break;  
    case 30:  ioctl(leds_fd, LEDS_PGM4_2_OFF);  
        break;  
    case 31:  ioctl(leds_fd, LEDS_PGM4_2_ON);  
        break;  
    case 40:  ioctl(leds_fd, LEDS_PGM4_3_OFF);  
        break;  
    case 41:  ioctl(leds_fd, LEDS_PGM4_3_ON);  
        break;  
    defautl :  
        break;  
    }  
  
    close(leds_fd);  
  
    return 0;  
}  