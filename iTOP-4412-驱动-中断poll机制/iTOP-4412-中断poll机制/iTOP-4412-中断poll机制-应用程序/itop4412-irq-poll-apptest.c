#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <poll.h>

int main(int argc, char **argv)
{
	int fd;
	unsigned char key_val;
	int ret;

	struct pollfd fds[1];
	
	fd = open("/dev/irq_test", O_RDWR);
	if (fd < 0)
	{
		printf("can't open!\n");
	}

	fds[0].fd     = fd;
	fds[0].events = POLLIN;  // POLLIN:普通或优先级带数据可读;
	while (1)
	{
		ret = poll(fds, 1, 5000);
		if (ret == 0)
		{
			printf("time out\n");
		}
		else
		{
			read(fd, &key_val, 1);
			if ( key_val == 0x01 )
				printf("press home button ! \n" );
			else if ( key_val == 0x02) 
				printf("press back button ! \n");
			else 
				;
		}
	}
	
	return 0;
}

