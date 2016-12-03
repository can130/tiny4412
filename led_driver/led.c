#include <linux/init.h>  
#include <linux/kernel.h>  
#include <linux/module.h>  
#include <linux/io.h>  
#include <linux/fs.h>  
#include <linux/ioctl.h>  
#include <linux/miscdevice.h>  
  
#define DEVICE_NAME     "tiny4412-leds"  
  
#define GPM4CON     (0x11000000+0x02E0)  
#define GPM4DAT     (0x11000000+0x02E4)  
  
#define GPM4_0  0  
#define GPM4_1  1  
#define GPM4_2  2  
#define GPM4_3  3  
  
#define GPM4_ON     0  
#define GPM4_OFF    1  
  
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
  
  
unsigned int *gpm4_con = NULL;  
unsigned int *gpm4_dat = NULL;  
  
  
/* 设置IO口输出电平 */  
static void set_gmp4_out(unsigned char cpm4_n, unsigned char status)   
{  
    unsigned int temp;  
      
    temp = readl(gpm4_dat);  
      
    if (status == GPM4_ON)   
        temp &=  ~(0x1<<cpm4_n);   
    else  
        temp |= 0x1<<cpm4_n;  
      
    writel(temp, gpm4_dat);  
}  
  
  
static long tiny4412_leds_ioctl(struct file *file, unsigned int cmd,  
                unsigned long arg)  
{  
    switch (cmd) {  
    case LEDS_PGM4_0_ON:  set_gmp4_out(GPM4_0, GPM4_ON);  
        break;  
    case LEDS_PGM4_0_OFF: set_gmp4_out(GPM4_0, GPM4_OFF);  
        break;  
    case LEDS_PGM4_1_ON:  set_gmp4_out(GPM4_1, GPM4_ON);  
        break;  
    case LEDS_PGM4_1_OFF: set_gmp4_out(GPM4_1, GPM4_OFF);  
        break;  
    case LEDS_PGM4_2_ON:  set_gmp4_out(GPM4_2, GPM4_ON);  
        break;  
    case LEDS_PGM4_2_OFF: set_gmp4_out(GPM4_2, GPM4_OFF);  
        break;  
    case LEDS_PGM4_3_ON:  set_gmp4_out(GPM4_3, GPM4_ON);  
        break;  
    case LEDS_PGM4_3_OFF: set_gmp4_out(GPM4_3, GPM4_OFF);  
        break;  
    defautl :  
        break;  
    }  
      
    return 0;  
}  
  
  
static const struct file_operations tiny4412_leds_fops = {  
    .owner      = THIS_MODULE,  
    .unlocked_ioctl = tiny4412_leds_ioctl,  
};  
  
  
static struct miscdevice tiny4412_leds_miscdev = {  
    .minor = MISC_DYNAMIC_MINOR,  
    .name = DEVICE_NAME,  
    .fops = &tiny4412_leds_fops,  
};  
  
  
static int tiny4412_leds_init(void)   
{  
    unsigned int data;    
    unsigned int ret;  
      
    //IO初始化，将物理地址映射为虚拟地址  
    gpm4_con = ioremap(GPM4CON, 4);  
    gpm4_dat = ioremap(GPM4DAT, 4);  
    if (!gpm4_con || !gpm4_con) {  
        printk("ioremap faild!\n");  
        goto error1;   
    }  
      
    /* 将GPM4[0]-GPM4[3]设置为输出 */  
    data = readl(gpm4_con);    
    data &= ~((0xf<<12)|(0xf<<8)|(0xf<<4)|(0xf<<0));   
    data |=  (0x1<<12)|(0x1<<8)|(0x1<<4)|(0x1<<0);   
    writel(data, gpm4_con);   
  
    ret = misc_register(&tiny4412_leds_miscdev);  //注册混杂设备驱动  
    if (ret) {  
        printk("misc_register faild!\n");  
        goto error2;  
    }  
      
    printk("tiny4412_leds_init!\n");  
      
    return 0;  
  
      
error2:  
    iounmap(gpm4_con);  
    iounmap(gpm4_dat);  
  
error1:  
    return -ENOMEM;  
          
}  
  
  
static void tiny4412_leds_exit(void)  
{  
    unsigned int data;  
  
    misc_deregister(&tiny4412_leds_miscdev);  
      
    data = readl(gpm4_dat);  
    data |=  (0x1<<3)|(0x1<<2)|(0x1<<1)|(0x1<<0);   
    writel(data, gpm4_dat);  
      
    iounmap(gpm4_con);  
    iounmap(gpm4_dat);  
    printk("tiny4412_leds_exit!\n");  
}  
  
  
MODULE_LICENSE("GPL");  
MODULE_AUTHOR("Chen Jinpeng");  
  
  
module_init(tiny4412_leds_init);  
module_exit(tiny4412_leds_exit);  
