#include <linux/module.h>
#include <linux/types.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/completion.h>

#include <linux/kernel.h>  
#include <linux/fs.h>  
#include <linux/init.h>  
#include <linux/delay.h>  
#include <linux/irq.h>  
#include <asm/uaccess.h>  
#include <asm/irq.h>  
#include <asm/io.h>  
#include <linux/module.h>  
#include <linux/device.h>         //class_create  
//#include <mach/regs-gpio.h>       //S3C2410_GPF1  
//#include <asm/arch/regs-gpio.h>    
//#include <mach/hardware.h>  
//#include <asm/hardware.h>  
#include <linux/interrupt.h>  //wait_event_interruptible  
#include <linux/poll.h>   //poll  
#include <linux/fcntl.h>  
  
  
/* 定义并初始化等待队列头 */  
static DECLARE_WAIT_QUEUE_HEAD(button_waitq);  
  
  
static struct class *fifthdrv_class;  
static struct device *fifthdrv_device;  
/*  
static struct pin_desc{  
    unsigned int pin;  
    unsigned int key_val;  
};  
  
static struct pin_desc pins_desc[4] = {  
        {S3C2410_GPF1,0x01},  
        {S3C2410_GPF4,0x02},  
        {S3C2410_GPF2,0x03},  
        {S3C2410_GPF0,0x04},  
};   
*/
static int ev_press = 0;  
  
/* 键值: 按下时, 0x01, 0x02, 0x03, 0x04 */  
/* 键值: 松开时, 0x81, 0x82, 0x83, 0x84 */  
static unsigned char key_val;  
int major;  
  
static struct fasync_struct *button_fasync;  
  
/* 用户中断处理函数 */  
static irqreturn_t buttons_irq(int irq, void *dev_id)  
{  
//    struct pin_desc *pindesc = (struct pin_desc *)dev_id;  
    unsigned int pinval;  
//   pinval = s3c2410_gpio_getpin(pindesc->pin);  
  
    if(pinval)  
    {  
        /* 松开 */  
//        key_val = 0x80 | (pindesc->key_val);  
    }  
    else  
    {  
        /* 按下 */  
//        key_val = pindesc->key_val;  
    }  
  
    ev_press = 1;                            /* 表示中断已经发生 */  
    wake_up_interruptible(&button_waitq);   /* 唤醒休眠的进程 */  
  
    /* 用kill_fasync函数告诉应用程序，有数据可读了  
     * button_fasync结构体里包含了发给谁(PID指定) 
     * SIGIO表示要发送的信号类型 
     * POLL_IN表示发送的原因(有数据可读了) 
     */  
    kill_fasync(&button_fasync, SIGIO, POLL_IN);  
    return IRQ_HANDLED;  
}  
static int fifth_drv_open(struct inode * inode, struct file * filp)  
{  
    /*  K1 ---- EINT1,K2 ---- EINT4,K3 ---- EINT2,K4 ---- EINT0 
     *  配置GPF1、GPF4、GPF2、GPF0为相应的外部中断引脚 
     *  IRQT_BOTHEDGE应该改为IRQ_TYPE_EDGE_BOTH 
     */  
//    request_irq(IRQ_EINT1, buttons_irq, IRQ_TYPE_EDGE_BOTH, "K1",&pins_desc[0]);  
//    request_irq(IRQ_EINT4, buttons_irq, IRQ_TYPE_EDGE_BOTH, "K2",&pins_desc[1]);  
//    request_irq(IRQ_EINT2, buttons_irq, IRQ_TYPE_EDGE_BOTH, "K3",&pins_desc[2]);  
//    request_irq(IRQ_EINT0, buttons_irq, IRQ_TYPE_EDGE_BOTH, "K4",&pins_desc[3]);  
    return 0;  
}  
  
static ssize_t fifth_drv_read(struct file *file, char __user *user, size_t size,loff_t *ppos)  
{  
    if (size != 1)  
            return -EINVAL;  
      
    /* 当没有按键按下时，休眠。 
     * 即ev_press = 0; 
     * 当有按键按下时，发生中断，在中断处理函数会唤醒 
     * 即ev_press = 1;  
     * 唤醒后，接着继续将数据通过copy_to_user函数传递给应用程序 
     */  
    wait_event_interruptible(button_waitq, ev_press);  
    copy_to_user(user, &key_val, 1);  
      
    /* 将ev_press清零 */  
    ev_press = 0;  
    return 1;     
}  
static ssize_t fifth_drv_write(struct file *file, const char __user *buf, size_t count, loff_t *f_pos)
{
    int result = 0;
    
    copy_from_user(&key_val, buf, 1); 
    printk(KERN_ERR "buff:%c", key_val);
    ev_press = 1;                            /* 表示中断已经发生 */  
    wake_up_interruptible(&button_waitq);   /* 唤醒休眠的进程 */  
    
    /* 用kill_fasync函数告诉应用程序，有数据可读了  
     * button_fasync结构体里包含了发给谁(PID指定) 
     * SIGIO表示要发送的信号类型 
     * POLL_IN表示发送的原因(有数据可读了) 
     */
    printk(KERN_ERR "%s %d\n", __FUNCTION__, __LINE__);  
    kill_fasync(&button_fasync, SIGIO, POLL_IN);  
    return result;  
}
 
static int fifth_drv_close(struct inode *inode, struct file *file)  
{  
//    free_irq(IRQ_EINT1,&pins_desc[0]);  
//    free_irq(IRQ_EINT4,&pins_desc[1]);  
//    free_irq(IRQ_EINT2,&pins_desc[2]);  
//    free_irq(IRQ_EINT0,&pins_desc[3]);  
    return 0;  
}  
  
static unsigned int fifth_drv_poll(struct file *file, poll_table *wait)  
{  
    unsigned int mask = 0;  
  
    /* 该函数，只是将进程挂在button_waitq队列上，而不是立即休眠 */  
    poll_wait(file, &button_waitq, wait);  
  
    /* 当没有按键按下时，即不会进入按键中断处理函数，此时ev_press = 0  
     * 当按键按下时，就会进入按键中断处理函数，此时ev_press被设置为1 
     */  
    if(ev_press)  
    {  
        mask |= POLLIN | POLLRDNORM;  /* 表示有数据可读 */  
    }  
  
    /* 如果有按键按下时，mask |= POLLIN | POLLRDNORM,否则mask = 0 */  
    return mask;    
}  
  
/* 当应用程序调用了fcntl(fd, F_SETFL, Oflags | FASYNC);  
 * 则最终会调用驱动的fasync函数，在这里则是fifth_drv_fasync 
 * fifth_drv_fasync最终又会调用到驱动的fasync_helper函数 
 * fasync_helper函数的作用是初始化/释放fasync_struct 
 */  
static int fifth_drv_fasync(int fd, struct file *filp, int on)  
{  
    return fasync_helper(fd, filp, on, &button_fasync);  
}  
  
/* File operations struct for character device */  
static const struct file_operations fifth_drv_fops = {  
    .owner      = THIS_MODULE,  
    .open       = fifth_drv_open,  
    .read       = fifth_drv_read,  
    .write      = fifth_drv_write,  
    .release    = fifth_drv_close,  
    .poll       = fifth_drv_poll,  
    .fasync     = fifth_drv_fasync,  
};  
  
  
/* 驱动入口函数 */  
static int fifth_drv_init(void)  
{  
    /* 主设备号设置为0表示由系统自动分配主设备号 */  
    major = register_chrdev(0, "fifth_drv", &fifth_drv_fops);  
  
    /* 创建fifthdrv类 */  
    fifthdrv_class = class_create(THIS_MODULE, "fifthdrv");  
  
    /* 在fifthdrv类下创建buttons设备，供应用程序打开设备*/  
    fifthdrv_device = device_create(fifthdrv_class, NULL, MKDEV(major, 0), NULL, "buttons");  
  
    return 0;  
}  
  
/* 驱动出口函数 */  
static void fifth_drv_exit(void)  
{  
    unregister_chrdev(major, "fifth_drv");  
    device_unregister(fifthdrv_device);  //卸载类下的设备  
    class_destroy(fifthdrv_class);      //卸载类  
}  
  
module_init(fifth_drv_init);  //用于修饰入口函数  
module_exit(fifth_drv_exit);  //用于修饰出口函数      
  
MODULE_AUTHOR("LWJ");  
MODULE_DESCRIPTION("Just for Demon");  
MODULE_LICENSE("GPL");  //遵循GPL协议  
