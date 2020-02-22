#include <linux/init.h>
#include <linux/module.h>
#include <linux/timer.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/input.h>
#include <linux/errno.h>
#include <asm/uaccess.h>
#include <linux/sched.h>
#include <linux/platform_device.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <asm/irq.h>
#include <linux/gpio.h>
#include <plat/gpio-cfg.h>
#include <mach/gpio.h>
#include <mach/gpio-exynos4.h>
#include <plat/regs-timer.h>
#include <linux/jiffies.h>
#define timer_DEBUG
#ifdef timer_DEBUG
#define DPRINTK(x...) printk("timer_CTL DEBUG:" x)
#else
#define DPRINTK(x...)
#endif

#define DRIVER_NAME "timer_list"
#define DEVICE_NAME "timer_list"
static int num,led;

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("TOPEET");

struct timer_list s_timer;

static int second_timer_handle(unsigned long arg)
{
    num++;
    mod_timer(&s_timer, jiffies + HZ);
//    printk("topeet \n ,%d",num);
    if(num==60){
	    led=1;
    }
    if(num==120){
	    led=0;
	    num=0;
    }  
    return led;
}

static long timer_ctl_ioctl(struct file *file,
		unsigned int cmd, unsigned long arg)
{
        switch (cmd) {
		case 0:
			gpio_direction_output(EXYNOS4_GPL2(0), 0);
			break;
		case 1:             
		        gpio_direction_output(EXYNOS4_GPL2(0), 1);
		        break;
		default:
			return -EINVAL;
	}
	return 0;
}

static ssize_t timer_ctl_read(struct file *files, unsigned int *buffer,
		size_t count, loff_t *ppos)
{
             int counter;
	     unsigned int ret;
             counter=led;	     
             ret=copy_to_user(buffer, &counter, sizeof(counter)); 
              if(ret>0)  
             {  
                    return -1;  
             }
                return 0;  	     
}


static int timer_ctl_release(struct inode *inode, struct file *file){
	del_timer(&s_timer);
	DPRINTK("Device Closed Success!\n");
	return 0;
}

static int timer_ctl_open(struct inode *inode, struct file *file){
	init_timer(&s_timer);
        s_timer.function = &second_timer_handle;
        s_timer.expires = jiffies + HZ;
  
        add_timer(&s_timer);       

	DPRINTK("Device Opened Success!\n");
	return nonseekable_open(inode,file);		
}

int timer_pm(bool enable)
{
	int ret = 0;
	printk("debug: timer_list PM return %d\r\n" , ret);
	return ret;
};

static struct file_operations timer_ctl_ops = {
	.owner = THIS_MODULE,
	.open = timer_ctl_open,
	.release = timer_ctl_release,
	.unlocked_ioctl 	= timer_ctl_ioctl,
	.read 		       = timer_ctl_read,		
};

static  struct miscdevice timer_ctl_dev = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = DEVICE_NAME,
	.fops = &timer_ctl_ops,
	
};

static int timer_ctl_probe(struct platform_device *pdv){
	int ret;
	char *banner = "timer_list Initialize\n";
	gpio_free(EXYNOS4_GPL2(0));
	printk(banner);
	
	ret = gpio_request(EXYNOS4_GPL2(0),"LED");
	if(ret){
		DPRINTK( "gpio_request EXYNOS4_GPL2(0) failed!\n");
		return ret;
	}
		
	gpio_direction_output(EXYNOS4_GPL2(0), 0);
	
	ret = misc_register(&timer_ctl_dev);
	if(ret<0)
	{
		printk("leds:register device failed!\n");
		goto exit;
	}
	return 0;

exit:
	misc_deregister(&timer_ctl_dev);
	return ret;	
}

static int timer_ctl_remove(struct platform_device *pdv){
	
	printk(KERN_EMERG "\tremove\n");
//	gpio_free(EXYNOS4_GPL2(0));
	misc_deregister(&timer_ctl_dev);
	return 0;
}

static void timer_ctl_shutdown(struct platform_device *pdv){
	
	;
}

static int timer_ctl_suspend(struct platform_device *pdv,pm_message_t pmt){
	
	DPRINTK("timer_list suspend:power off!\n");
	return 0;
}

static int timer_ctl_resume(struct platform_device *pdv){
	
	DPRINTK("timer_list resume:power on!\n");
	return 0;
}

struct platform_driver timer_ctl_driver = {
	.probe = timer_ctl_probe,
	.remove = timer_ctl_remove,
	.shutdown = timer_ctl_shutdown,
	.suspend = timer_ctl_suspend,
	.resume = timer_ctl_resume,
	.driver = {
		.name = DRIVER_NAME,
		.owner = THIS_MODULE,
	}
};

static int timer_ctl_init(void)
{
	int DriverState;
	
	
	printk(KERN_EMERG "timer_ctl WORLD enter!\n");
	DriverState = platform_driver_register(&timer_ctl_driver);
	
	printk(KERN_EMERG "\tDriverState is %d\n",DriverState);
	return 0;
}


static void timer_ctl_exit(void)
{
	
	printk(KERN_EMERG "timer_ctl WORLD exit!\n");
	platform_driver_unregister(&timer_ctl_driver);	
}

module_init(timer_ctl_init);
module_exit(timer_ctl_exit);
