#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <stdint.h>
#include <termios.h>
//#include <android/log.h>
//#include <sys/ioctl.h>

int main(int argc,char *argv[]){
	int fd,i,j;
	char buffer[512];
	int len=0, r=0;

	memset(buffer,0,sizeof(buffer));
		printf("adc ready!\n");
	
	if((fd = open(argv[1], O_RDWR|O_NOCTTY|O_NDELAY))<0){
		printf("open %s err!\n",argv[1]);
		return -1;
	}
		
	printf("open %s success!\n",argv[1]);
	ioctl(fd,0,atoi(argv[2]));
	len=read(fd,buffer,10);	
	if(len == 0)
		printf("return null\n");
	else{
		r = atoi(buffer);
		printf("adc org value is %d\n",r);
		r = (int)(r*10000/4095);	//Datas  transition to Res
		printf("res value is %d\n",r);		
	}
}
















