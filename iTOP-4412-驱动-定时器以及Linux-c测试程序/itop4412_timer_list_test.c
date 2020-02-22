#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h> 
#include <stdlib.h>
#include <sys/ioctl.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>
#include <termios.h>

int main(void)
{
    int fd, i;
    int data;
    int t=0;
    fd = open("/dev/timer_list",O_RDWR|O_NOCTTY);
    if (fd < 0)
    {
        printf("open /dev/second error\n");
    }
    while(1)
    {
      read(fd, &data, sizeof(data));
      ioctl(fd,data);
      printf("led jishu miao is %d,\n",t);//定时一分钟led亮和灭
      t++;
      if(t==60)
      t=0;
      sleep(1);
       }
    close(fd);
}