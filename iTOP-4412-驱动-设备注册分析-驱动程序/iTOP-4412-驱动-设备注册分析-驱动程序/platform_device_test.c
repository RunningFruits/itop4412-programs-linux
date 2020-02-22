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

static void leds_release(struct device * dev)
{
}

static struct platform_device leds_device = {
    .name         = "my_code_led",
    .id       = -1,
	.dev = { 
    	.release = leds_release, ////必须向内核提供一个release函数,否则释放的时候会报错
	},
};


static int __init iTop4412_platform_add_device_init(void)
{
	return platform_device_register(&leds_device);
}

static void __exit iTop4412_platform_add_device_exit(void)
{
	platform_device_unregister(&leds_device);
}

module_init(iTop4412_platform_add_device_init);
module_exit(iTop4412_platform_add_device_exit);
MODULE_LICENSE("GPL");