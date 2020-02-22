#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
//定义蜂鸣器打开的操作
#define PWM_OPEN		1
//定义蜂鸣器关闭的操作
#define PWM_STOP			0
static void close_bell(void);
static void open_bell(void);
static void set_bell_freq(int freq);
static void stop_bell(void);
int main(int argc, char **argv)
{
	//设置蜂鸣器的频率的初始值为1000
	int freq = 1000 ;
	//打开蜂鸣器
	open_bell();
	stop_bell();
	while( 1 )
	{
		while(1){
		//设置蜂鸣器
		set_bell_freq(freq);
		//如果频率小于20000
		if(freq < 20000){
			//自加
			freq+=100 ;
			printf( "\tFreq = %d\n", freq );
			//跳出到另外一个循环
			if(freq == 20000){
				break ;
			}
		}
		}
		
		while(1){
			//设置蜂鸣器	
			set_bell_freq(freq);
			//如果频率大于1000
			if(freq > 1000)
				//自减
				freq-=100 ;
			printf( "\tFreq = %d\n", freq );
			if(freq == 1000){
				break ;
			}			
		}
		//周而复始的执行，于是蜂鸣器就会像唱歌一样
	}
}

static int fd = -1;
//打开蜂鸣器
static void open_bell(void)
{
	//打开设备
	fd = open("/dev/pwm", 0);
	//如果打开小于0表示失败
	if (fd < 0) {
		perror("open pwm_buzzer device");
		exit(1);
	}

	//初始化蜂鸣器的时候先关闭，不让它叫
	atexit(close_bell);
}

//关闭峰鸣器
static void close_bell(void)
{
	if (fd >= 0) {
		//关闭蜂鸣器
		ioctl(fd, PWM_STOP);
		if (ioctl(fd, 2) < 0) {
			perror("ioctl 2:");
		}
		close(fd);
		fd = -1;
	}
}
//设置蜂鸣器的频率
static void set_bell_freq(int freq)
{
	//设置频率
	int ret = ioctl(fd, PWM_OPEN, freq);
	if(ret < 0) {
		perror("set the frequency of the buzzer");
		exit(1);
	}
}
//停止蜂鸣器
static void stop_bell(void)
{
	//让蜂鸣器停止叫
	int ret = ioctl(fd, PWM_STOP);
	if(ret < 0) {
		perror("stop the buzzer");
		exit(1);
	}
	if (ioctl(fd, 2) < 0) {
		perror("ioctl 2:");
	}
}