﻿#include <linux/init.h>  
#include <linux/kernel.h>  
#include <linux/module.h>  
#include <linux/io.h>  
#include <linux/fs.h>  
#include <linux/ioctl.h>  
#include <linux/miscdevice.h>  
#include <asm/uaccess.h>

#define DEVICE_NAME     "tiny4412-ds1302"  
  
  
#define GPX3CON     (0x11000000+0x0C60)  
#define GPX3DAT     (0x11000000+0x0C64)    

#define DS1302_RST     GPX3_2
#define DS1302_DAT     GPX3_3
#define DS1302_CLK     GPX3_4

#define GPX3_2  2
#define GPX3_3  3  
#define GPX3_4  4  
  
  

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



unsigned int *Ds1302_con = NULL;  
unsigned int *Ds1302_dat = NULL;  
  
  
/* 设置IO口输出电平 */  
static void set_dat_out()   
{  
    unsigned int temp;  
      
    temp = readl(Ds1302_con);  
      
    temp &=  ~(0xf<<DS1302_DAT*4);     
    temp |=  0x1<<DS1302_DAT*4;   
    writel(temp, Ds1302_con);  
}  

static void set_dat_in()   
{  
    unsigned int temp;  
      
    temp = readl(Ds1302_con);  
      
    temp &=  ~(0xf<<DS1302_DAT*4);     	
    writel(temp, Ds1302_con);  
}

static int ds1302_dat_read()
{
	unsigned int temp;  
      
    temp = readl(Ds1302_dat);  
      
    return (temp&(0x1<<DS1302_DAT));
}

static void ds1302_write(unsigned int pin,unsigned int status)
{
	unsigned int temp;  
      
    temp = readl(Ds1302_dat);  
      
    if (status == 0)   
        temp &=  ~(0x1<<pin);   
    else  
        temp |= 0x1<<pin;  
      
    writel(temp, Ds1302_dat); 
}
  
  
  
/* 发送一个字节到DS1302通信总线上 */
void DS1302ByteWrite(unsigned char dat)
{
    unsigned char mask;
	
    set_dat_out();
	
    for (mask=0x01; mask!=0; mask<<=1)  //低位在前，逐位移出
    {
        if ((mask&dat) != 0) //首先输出该位数据
            ds1302_write(DS1302_DAT,1);//DS1302_IO = 1;
        else
            ds1302_write(DS1302_DAT,0);//DS1302_IO = 0;
        ds1302_write(DS1302_CLK,1);  //DS1302_CK = 1;       //然后拉高时钟
        ds1302_write(DS1302_CLK,0);    //DS1302_CK = 0;       //再拉低时钟，完成一个位的操作
    }
     ds1302_write(DS1302_DAT,1);//DS1302_IO = 1;           //最后确保释放IO引脚
}


/* 由DS1302通信总线上读取一个字节 */
unsigned char DS1302ByteRead()
{
    unsigned char mask;
    unsigned char dat = 0;
    
	set_dat_in();
	
    for (mask=0x01; mask!=0; mask<<=1)  //低位在前，逐位读取
    {
        if (ds1302_dat_read() != 0)  //首先读取此时的IO引脚，并设置dat中的对应位
        {
            dat |= mask;
        }
        ds1302_write(DS1302_CLK,1);//DS1302_CK = 1;       //然后拉高时钟
        ds1302_write(DS1302_CLK,0);//DS1302_CK = 0;       //再拉低时钟，完成一个位的操作
    }
    return dat;              //最后返回读到的字节数据
}

/* 用单次写操作向某一寄存器写入一个字节，reg-寄存器地址，dat-待写入字节 */
void DS1302SingleWrite(unsigned char reg, unsigned char dat)
{
    ds1302_write(DS1302_RST,1);//DS1302_CE = 1;                      //使能片选信号
    DS1302ByteWrite((reg<<1)|0x80);  //发送写寄存器指令
    DS1302ByteWrite(dat);              //写入字节数据
    ds1302_write(DS1302_RST,0);//DS1302_CE = 0;                      //除能片选信号
}

/* 用单次读操作从某一寄存器读取一个字节，reg-寄存器地址，返回值-读到的字节 */
unsigned char DS1302SingleRead(unsigned char reg)
{
    unsigned char dat;
    
    ds1302_write(DS1302_RST,1);//DS1302_CE = 1;                      //使能片选信号
    DS1302ByteWrite((reg<<1)|0x81);  //发送读寄存器指令
    dat = DS1302ByteRead();           //读取字节数据
    ds1302_write(DS1302_RST,0);//DS1302_CE = 0;                      //除能片选信号
    
    return dat;
}
/* 用突发模式连续写入8个寄存器数据，dat-待写入数据指针 */
void DS1302BurstWrite(unsigned char *dat)
{
    unsigned char i;
    
    ds1302_write(DS1302_RST,1);//DS1302_CE = 1;
    DS1302ByteWrite(0xBE);  //发送突发写寄存器指令
	
    for (i=0; i<8; i++)     //连续写入8字节数据
    {
        DS1302ByteWrite(dat[i]);
    }
    ds1302_write(DS1302_RST,0);// DS1302_CE = 0;
}

static void DS1302BurstRead(unsigned char *dat)
{
    unsigned char i;
    
    ds1302_write(DS1302_RST,1);//DS1302_CE = 1;
    DS1302ByteWrite(0xBF);  //发送突发读寄存器指令
    for (i=0; i<8; i++)     //连续读取8个字节
    {
        dat[i] = DS1302ByteRead();
    }
    ds1302_write(DS1302_RST,0);// DS1302_CE = 0;
}
static void SetRealTime(struct sTime *time)
{
    unsigned char buf[8];
    
    buf[7] = 0;
    buf[6] = time->year;
    buf[5] = time->week;
    buf[4] = time->mon;
    buf[3] = time->day;
    buf[2] = time->hour;
    buf[1] = time->min;
    buf[0] = time->sec;
    DS1302BurstWrite(buf);
}

static void InitDS1302()
{
    unsigned char dat;
    struct sTime InitTime[] = {  //2013年10月8日 12:30:00 星期二
        0x2013,0x10,0x08, 0x12,0x30,0x00, 0x02
    };
    
    ds1302_write(DS1302_RST,0);//DS1302_CE = 0;  //初始化DS1302通信引脚
    ds1302_write(DS1302_CLK,0);//DS1302_CK = 0;
    dat = DS1302SingleRead(0);  //读取秒寄存器
    if ((dat & 0x80) != 0)      //由秒寄存器最高位CH的值判断DS1302是否已停止
    {
        DS1302SingleWrite(7, 0x00);  //撤销写保护以允许写入数据
        SetRealTime(&InitTime);      //设置DS1302为默认的初始时间
    }
}

static long tiny4412_ds1302_ioctl(struct file *file, unsigned int cmd,  
                unsigned long arg)  
{  
    switch (cmd) {  
    case DS1302_READ_TIME:  ;  
        break;  
    defautl :  
        break;  
    }  
      
    return 0;  
}  
 
static int ds1302_read(struct file *filp, char __user *buff, size_t count, loff_t *offp)
{
     unsigned long err; 
     unsigned char buf[8];
	
	 struct sTime time;

     DS1302BurstRead(buf);
     time.year = buf[6] + 0x2000;
     time.mon  = buf[4];
     time.day  = buf[3];
     time.hour = buf[2];
     time.min  = buf[1];
     time.sec  = buf[0];
     time.week = buf[5];
	 
	 
	 	err=copy_to_user((void *)buff, (const void *)(&time), min(sizeof(time), count));
        return err ? -EFAULT : min(sizeof(time), count);
}
  
static const struct file_operations tiny4412_ds1302_fops = {  
    .owner      = THIS_MODULE,  
    .unlocked_ioctl = tiny4412_ds1302_ioctl,  
	.read	= ds1302_read,
};  
  
  
static struct miscdevice tiny4412_ds1302_miscdev = {  
    .minor = MISC_DYNAMIC_MINOR,  
    .name = DEVICE_NAME,  
    .fops = &tiny4412_ds1302_fops,   
};  
  

static int tiny4412_ds1302_init(void)   
{  
    unsigned int data;    
    unsigned int ret;  
      
    //IO初始化，将物理地址映射为虚拟地址  
    Ds1302_con = ioremap(GPX3CON, 4);  
    Ds1302_dat = ioremap(GPX3DAT, 4);  
    if (!Ds1302_con || !Ds1302_dat) {  
        printk("ioremap faild!\n");  
        goto error1;   
    }  
      
    /* 将GPM4[0]-GPM4[3]设置为输出 */  
    data = readl(Ds1302_con);    
    data &= ~((0xf<<8)|(0xf<<12)|(0xf<<16));   //gpx3.234  
    data |=  (0x1<<8)|(0x1<<12)|(0x1<<16);   
    writel(data, Ds1302_con);   
  
    ret = misc_register(&tiny4412_ds1302_miscdev);  //注册混杂设备驱动  
    if (ret) {  
        printk("misc_register faild!\n");  
        goto error2;  
    }  
    
	InitDS1302();
	
    printk("tiny4412_leds_init!\n");  
      
    return 0;  
  
      
error2:  
    iounmap(Ds1302_con);  
    iounmap(Ds1302_dat);  
  
error1:  
    return -ENOMEM;  
          
}  
  
  
static void tiny4412_ds1302_exit(void)  
{  
    unsigned int data;  
  
    misc_deregister(&tiny4412_ds1302_miscdev);  
      
    data = readl(Ds1302_dat);  
    data |=  (0x1<<3)|(0x1<<2)|(0x1<<1)|(0x1<<0);   
    writel(data, Ds1302_dat);  
      
    iounmap(Ds1302_con);  
    iounmap(Ds1302_dat);  
    printk("tiny4412_leds_exit!\n");  
}  
  
  
MODULE_LICENSE("GPL");  
MODULE_AUTHOR("chuncanL");  
  
  
module_init(tiny4412_ds1302_init);  
module_exit(tiny4412_ds1302_exit);  
