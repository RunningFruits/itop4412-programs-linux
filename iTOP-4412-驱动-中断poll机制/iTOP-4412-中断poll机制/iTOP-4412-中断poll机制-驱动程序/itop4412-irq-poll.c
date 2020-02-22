/*以后写驱动可以讲头文件一股脑的加载代码前面*/
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <mach/gpio.h>
#include <plat/gpio-cfg.h>
#include <linux/miscdevice.h>
#include <linux/platform_device.h>
#include <mach/regs-gpio.h>
#include <asm/io.h>
#include <linux/regulator/consumer.h>
#include <linux/delay.h>

/*中断函数头文件*/
#include <linux/interrupt.h>
#include <linux/irq.h>

#define IRQ_DEBUG
#ifdef IRQ_DEBUG
#define DPRINTK(x...) printk("IRQ_CTL DEBUG:" x)
#else
#define DPRINTK(x...)
#endif

#define DRIVER_NAME "irq_test"


//添加头文件
#include <linux/poll.h>
#include <linux/sched.h>

//添加的定义：
static volatile int press = 0;
static unsigned char key_val;
static DECLARE_WAIT_QUEUE_HEAD(key_button);

//中断函数
static irqreturn_t eint9_interrupt(int irq, void *dev_id) {
	press = 1;                  // 表示中断发生了 
	key_val = 0x01;  //read返回值，
    wake_up_interruptible(&key_button);   // 唤醒休眠的进程 
	return IRQ_RETVAL(IRQ_HANDLED);     
}

static irqreturn_t eint10_interrupt(int irq, void *dev_id) {
	press = 1;                  // 表示中断发生了 
	key_val = 0x02;
    wake_up_interruptible(&key_button);   /* 唤醒休眠的进程 */
	return IRQ_RETVAL(IRQ_HANDLED);   
}


static int itop_irq_open(struct inode *inode, struct file *file)
{
	return 0;
}

static int itop_irq_close(struct inode *inode, struct file *file)
{
	printk(" %s  ! ! !\n",__FUNCTION__);
	return 0;
}

ssize_t itop_irq_read(struct file *file, char __user *buf, size_t size, loff_t *ppos)
{
	if (size != 1)
		return -EINVAL;
	//如果没有按键动作, 休眠 
	wait_event_interruptible(key_button, press); 
	// 如果有按键动作, 返回键值 
	copy_to_user(buf, &key_val, 1);
	press = 0;
	return 1;
}

static unsigned itop_irq_poll(struct file *file, poll_table *wait)
{
	unsigned int mask = 0;
	poll_wait(file, &key_button, wait); // 不会立即休眠
	if (press)
	mask |= POLLIN | POLLRDNORM;  // POLLIN:普通或优先级带数据可读; POLLRDNORM:normal普通数据可读  
	return mask;  //当超时,就返给应用层为0 ,被唤醒了就返回POLLIN | POLLRDNORM ;
}

static struct file_operations itop_irq_fops = {
    .owner   =  THIS_MODULE,    //这是一个宏，推向编译模块时自动创建的__this_module变量 
    .open    =  itop_irq_open,     
	.read	 =	itop_irq_read,	   
	.release =  itop_irq_close,
	.poll    =  itop_irq_poll,
};

static struct miscdevice itop_irq_dev = {
	.minor	= MISC_DYNAMIC_MINOR,
	.fops	= &itop_irq_fops,
	.name	= "irq_test",
};

// probe 函数
static int irq_probe(struct platform_device *pdev)
{
		int ret;
        char *banner = "irq_test Initialize\n";
		printk(" %s  ! ! !\n",__FUNCTION__);
        printk(banner);
		
        ret = gpio_request(EXYNOS4_GPX1(1), "EINT9");
        if (ret) {
                printk("%s: request GPIO %d for EINT9 failed, ret = %d\n", DRIVER_NAME,
                        EXYNOS4_GPX1(1), ret);
                return ret;
        }
        s3c_gpio_cfgpin(EXYNOS4_GPX1(1), S3C_GPIO_SFN(0xF));
        s3c_gpio_setpull(EXYNOS4_GPX1(1), S3C_GPIO_PULL_UP);
        gpio_free(EXYNOS4_GPX1(1));

        ret = gpio_request(EXYNOS4_GPX1(2), "EINT10");
        if (ret) {
                printk("%s: request GPIO %d for EINT10 failed, ret = %d\n", DRIVER_NAME,
                        EXYNOS4_GPX1(2), ret);
                return ret;
        }
        s3c_gpio_cfgpin(EXYNOS4_GPX1(2), S3C_GPIO_SFN(0xF));
        s3c_gpio_setpull(EXYNOS4_GPX1(2), S3C_GPIO_PULL_UP);
        gpio_free(EXYNOS4_GPX1(2));


        ret = request_irq(IRQ_EINT(9), eint9_interrupt,
                        IRQ_TYPE_EDGE_FALLING /*IRQF_TRIGGER_FALLING*/, "eint9", pdev);
        if (ret < 0) {
                printk("Request IRQ %d failed, %d\n", IRQ_EINT(9), ret);
                goto exit;
        }

        ret = request_irq(IRQ_EINT(10), eint10_interrupt,
                        IRQ_TYPE_EDGE_FALLING /*IRQF_TRIGGER_FALLING*/, "eint10", pdev);
        if (ret < 0) {
                printk("Request IRQ %d failed, %d\n", IRQ_EINT(10), ret);
                goto exit;
        }
	
		ret = misc_register(&itop_irq_dev);
        return 0;

exit:
        return ret;
}

static int irq_remove (struct platform_device *pdev)
{
        printk(" %s  ! ! !\n",__FUNCTION__);
		misc_deregister(&itop_irq_dev);
		free_irq(IRQ_EINT(9),pdev);
		free_irq(IRQ_EINT(10),pdev);
		
		return 0;
}

static int irq_suspend (struct platform_device *pdev, pm_message_t state)
{
        printk(" %s  ! ! !\n",__FUNCTION__);
		DPRINTK("irq suspend:power off!\n");
        return 0;
}

static int irq_resume (struct platform_device *pdev)
{	
		printk(" %s  ! ! !\n",__FUNCTION__);
        DPRINTK("irq resume:power on!\n");
        return 0;
}

static struct platform_driver irq_driver = {
        .probe = irq_probe,
        .remove = irq_remove,
        .suspend = irq_suspend,
        .resume = irq_resume,
        .driver = {
                .name = DRIVER_NAME,
                .owner = THIS_MODULE,
        },
};

static void __exit irq_test_exit(void)
{
		printk(" %s  ! ! !\n",__FUNCTION__);
		platform_driver_unregister(&irq_driver);
}

static int __init irq_test_init(void)
{
		printk(" %s  ! ! !\n",__FUNCTION__);
        return platform_driver_register(&irq_driver);
}

module_init(irq_test_init);
module_exit(irq_test_exit);

MODULE_LICENSE("Dual BSD/GPL");