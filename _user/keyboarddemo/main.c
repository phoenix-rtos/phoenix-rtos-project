#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <poll.h>
#include <unistd.h>

int main(void)
{
	int fd = open("/dev/kbd", O_RDONLY);

	unsigned char c;
	int rv;

	while (1) {
		if ((rv = read(fd, &c, 1)) > 0) {
			if (c & (1 << 8)) {
				printf("(shift) ");
			}
			if (c & 0x80) {
				c &= 0x7f;
				printf("%u - released\n", c);
			}
			else
				printf("%u - pressed\n", c);
			fflush(stdout);
		}
		if (rv < 0)
			printf("%s", strerror(errno));
		usleep(200);
	}
}
