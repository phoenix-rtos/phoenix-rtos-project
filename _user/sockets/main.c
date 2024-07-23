/*
 * Phoenix-RTOS
 *
 * Sockets
 *
 * Example use of UNIX socket API, specifically of non-blocking operations
 *
 * Copyright 2024 Phoenix Systems
 * Author: Adam Greloch
 *
 * This file is part of Phoenix-RTOS.
 *
 * %LICENSE%
 */

#define SOCK_PATH "/tmp/0.sock"
#define BUF_SIZE 16

#include <unistd.h>
#include <sys/un.h>
#include <string.h>
#include <poll.h>
#include "err.h"

int main(void)
{
  unlink(SOCK_PATH);

  int sock, datasock;
  int k;
  struct sockaddr_un addr;
  pid_t pid;
  char buf[BUF_SIZE];

  memset(&addr, 0, sizeof(struct sockaddr_un));

  addr.sun_family = AF_UNIX;
  strncpy(addr.sun_path, SOCK_PATH, sizeof(addr.sun_path) - 1);

  CHECK_ERRNO(pid = fork());

  if (pid == 0) {
    CHECK_ERRNO(datasock = socket(AF_UNIX, SOCK_STREAM | SOCK_NONBLOCK, PF_UNSPEC));
    sleep(1);
    k = connect(datasock, (const struct sockaddr *) &addr, sizeof(struct sockaddr_un));
    if (k < 0 && errno == EINPROGRESS)
      printf("connect() didn't block!\n");

    int timeout_msecs = 500;
    struct pollfd pfd[1];

    pfd[0].fd = datasock;
    pfd[0].events = POLLOUT;

    CHECK_ERRNO(poll(pfd, 1, timeout_msecs));

    if (pfd[0].revents & POLLOUT) {
      printf("poll(): socket ready to write\n");
    }
    if (pfd[0].revents & POLLHUP) {
      printf("poll(): connection disconnected\n");
    }

  } else {
    CHECK_ERRNO(sock = socket(AF_UNIX, SOCK_STREAM | SOCK_NONBLOCK, PF_UNSPEC));
    CHECK_ERRNO(bind(sock, (const struct sockaddr *) &addr, sizeof(struct sockaddr_un)));
    CHECK_ERRNO(listen(sock, 0));

    datasock = accept(sock, NULL, NULL);
    if (datasock < 0) {
      printf("accept() didn't block and returned: %s\n", strerror(errno));
    } else {
      printf("accept(): connection estabilished\n");
    }

    CHECK_ERRNO(read(datasock, buf, BUF_SIZE));
    buf[BUF_SIZE-1] = 0;
    printf("read(): got '%s'\n", buf);

    wait(NULL);
  }

	return 0;
}
