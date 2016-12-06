﻿#include <stdio.h>     
#include <sys/types.h>    
#include <sys/stat.h>  
#include <fcntl.h>  
#include <linux/ioctl.h>  
  
  
#define DEVICE_NAME     "/dev/tiny4412-ds1302"  
  
//幻数  
#define DS1302_MAGIC  'a'  
#define DS1302_READ_TIME      _IO(DS1302_MAGIC, 1) 
 

struct sTime {  //日期时间结构体定义
     unsigned int  year;  //年
     unsigned char mon;   //月
     unsigned char day;   //日
     unsigned char hour;  //时
     unsigned char min;   //分
     unsigned char sec;   //秒
     unsigned char week;  //星期
};
  
int main (int argc, char *argv[])   
{  
    int leds_fd = 0;  
    int cmd;  
	char buf[10];
	struct sTime time;     
    if  (argc < 2) {  
        printf("please entry the correct operation parameter! \n");  
        return 0;  
    }  
  
    leds_fd = open(DEVICE_NAME, O_RDWR);  
    if (leds_fd == -1) {  
        printf("open device faild! \n");  
        return 0;  
    }   
  
    read(leds_fd,time,sizeof(time));  
	printf("%d:%d\n",time.min,time.sec);
  
    close(leds_fd);  
  
    return 0;  
}  
