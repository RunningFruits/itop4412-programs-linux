
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <poll.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>

int fd;
void my_signal_fun(int signum)
{
	unsigned char key_val;
	read(fd, &key_val, 1);
			if ( key_val == 0x01 )
				printf("press home button ! \n" );
			else if ( key_val == 0x02) 
				printf("press back button ! \n");
			else 
				;

}

int main(int argc, char **argv)
{
	unsigned char key_val;
	int ret;
	int Oflags;

	signal(SIGIO, my_signal_fun);
	
	fd = open("/dev/irq_test", O_RDWR);
	if (fd < 0)
	{
		printf("can't open!\n");
	}

	fcntl(fd, F_SETOWN, getpid());
	
	Oflags = fcntl(fd, F_GETFL); 
	
	fcntl(fd, F_SETFL, Oflags | FASYNC);


	while (1)
	{
		sleep(1000);
	}
	
	return 0;
}


























