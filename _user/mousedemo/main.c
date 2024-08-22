#include <stdlib.h>
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
	int fd = open("/dev/mouse", O_RDONLY);

	unsigned char c[3];
	int rv;

	struct pollfd fds[1];

	fds[0].fd = fd;
	fds[0].events = POLLIN;

	while (1) {
		rv = poll(fds, 1, 1000);
		if (rv < 0) {
			printf("poll error: %s\n", strerror(errno));
			exit(1);
		}
		if (fds[0].revents & POLLIN) {
			if ((rv = read(fd, &c, 3)) > 0) {
				for (int i = 0; i < 3; i++) {
					if (i < 2)
						printf("%02x ", c[i]);
					else
						printf("%02x\n", c[i]);
					fflush(stdout);
				}
			}
			else {
				printf("read: %s\n", strerror(errno));
			}
		}
		fflush(stdout);
	}
}
