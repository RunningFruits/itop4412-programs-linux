#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/fb.h>
#include <linux/backlight.h>
#include <linux/err.h>
#include <linux/pwm.h>
#include <linux/slab.h>
#include <linux/miscdevice.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <mach/gpio.h>
#include <plat/gpio-cfg.h>

//定义设备的名字为 pwm
#define DEVICE_NAME				"pwm"

#define PWM_IOCTL_SET_FREQ		1
#define PWM_IOCTL_STOP			0

#define NS_IN_1HZ				(1000000000UL)

//蜂鸣器PWM_ID  0 
#define BUZZER_PWM_ID			0
//蜂鸣器GPIO配置
#define BUZZER_PMW_GPIO			EXYNOS4_GPD0(0)

//定义一个结构体指针
static struct pwm_device *pwm4buzzer;
//定义一个结构体信号量指针,因为信号量与锁的机制差不多
//Mutex是一把钥匙，一个人拿了就可进入一个房间，出来的时候把钥匙交给队列的第一个。一般的用法是用于串行化对critical section代码的访问，保证这段代码不会被并行的运行。
//Semaphore是一件可以容纳N人的房间，如果人不满就可以进去，如果人满了，就要等待有人出来。对于N=1的情况，称为binary semaphore。一般的用法是，用于限//制对于某一资源的同时访问。
static struct semaphore lock;

//设置频率
static void pwm_set_freq(unsigned long freq) {
//PWM的占空比的配置
	//printk("pwm_set_freq start");
	int period_ns = NS_IN_1HZ / freq;

	pwm_config(pwm4buzzer, period_ns / 2, period_ns); 
	pwm_enable(pwm4buzzer);
	//配置相应的GPIO,将蜂鸣器IO配置成PWM输出模式
	s3c_gpio_cfgpin(BUZZER_PMW_GPIO, S3C_GPIO_SFN(2));
}
//stop方法函数，来源于operations结构体
static  void pwm_stop(void) {
	//printk("pwm_stop start");
	s3c_gpio_cfgpin(BUZZER_PMW_GPIO, S3C_GPIO_OUTPUT);

	pwm_config(pwm4buzzer, 0, NS_IN_1HZ / 100);
	pwm_disable(pwm4buzzer);
}

//open方法函数，来源于operations结构体，主要打开pwm的操作
static int iTop4412_pwm_open(struct inode *inode, struct file *file) {
	//printk("iTop4412_pwm_open start");
	if (!down_trylock(&lock)) //尝试加锁，如果失败返回0
		return 0;
	else
		return -EBUSY;
}

//close方法函数，来源于operations结构体，主要是关闭pwm操作
static int iTop4412_pwm_close(struct inode *inode, struct file *file) {
	//printk("iTop4412_pwm_close start");
	up(&lock);
	return 0;
}
//控制io口方法函数，来源于operations结构体,其实就是上层系统调用传入一条命令，//驱动识别命令，然后执行相应过程。
static long iTop4412_pwm_ioctl(struct file *filep, unsigned int cmd,
		unsigned long arg)
{
	//printk("iTop4412_pwm_ioctl start");
	switch (cmd) {
		case PWM_IOCTL_SET_FREQ:
			if (arg == 0)
				return -EINVAL;
			pwm_set_freq(arg);
			break;

		case PWM_IOCTL_STOP:
		default:
			pwm_stop();
			break;
	}

	return 0;
}

//这就是我们要看的结构体了，其实这个结构体的定义在另一个.h当中，看看它的初始//化方式，跟我们上面那个程序的分析基本上是一样的。对应的函数名（也就是函数的//首地址）赋值给对应的结构体成员，实现了整个结构体的初始化，这样的方法类似于//C++和JAVA等高级语言的操作。
static  struct file_operations iTop4412_pwm_ops = {
	.owner			= THIS_MODULE,  			//表示本模块拥有
	.open			= iTop4412_pwm_open,		//表示调用open函数
	.release		= iTop4412_pwm_close,         //…
	.unlocked_ioctl	= iTop4412_pwm_ioctl,
};

//杂类设备的注册
static struct miscdevice iTop4412_misc_dev = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = DEVICE_NAME,
	.fops = &iTop4412_pwm_ops,
};
//pwm设备初始化,设备在被insmod插入模块到内核的过程中会调用这个函数
static int __init iTop4412_pwm_dev_init(void) {
	//printk("iTop4412_pwm_dev_init start");
	int ret;
	gpio_free(BUZZER_PMW_GPIO);
	
	ret = gpio_request(BUZZER_PMW_GPIO, DEVICE_NAME);
	if (ret) {
		printk("request GPIO %d for pwm failed\n", BUZZER_PMW_GPIO);
		return ret;
	}

	gpio_set_value(BUZZER_PMW_GPIO, 0);
	s3c_gpio_cfgpin(BUZZER_PMW_GPIO, S3C_GPIO_OUTPUT);

	pwm4buzzer = pwm_request(BUZZER_PWM_ID, DEVICE_NAME);
	if (IS_ERR(pwm4buzzer)) {
		printk("request pwm %d for %s failed\n", BUZZER_PWM_ID, DEVICE_NAME);
		return -ENODEV;
	}

	pwm_stop();

	sema_init(&lock, 1);
	ret = misc_register(&iTop4412_misc_dev);

	printk(DEVICE_NAME "\tinitialized\n");

	return ret;
}
//设备在被卸载rmmod的过程中会调用这个函数
static void __exit iTop4412_pwm_dev_exit(void) {
	//printk("iTop4412_pwm_exit start");
	pwm_stop();

	misc_deregister(&iTop4412_misc_dev);
	gpio_free(BUZZER_PMW_GPIO);
}

//模块初始化
module_init(iTop4412_pwm_dev_init);
//销毁模块
module_exit(iTop4412_pwm_dev_exit);
//声明GPL协议
MODULE_LICENSE("GPL");
//作者：yangyuanxin
MODULE_AUTHOR("Yangyuanxin");
//描述:三星PWM设备
MODULE_DESCRIPTION("Exynos4 PWM Driver");