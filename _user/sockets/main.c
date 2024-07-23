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
#include <fcntl.h>
#include <stdbool.h>
#include "err.h"

/* use to check whether accept() is indeed non-blocking */
bool client_noop = false;

/* enable to use fcntl for setting up non-blocking flag  */
bool use_fcntl = true;

bool nonblocking_client = true;
bool nonblocking_server = true;

int create_socket(bool nonblocking)
{
	int sock, flags;
	if (nonblocking && use_fcntl) {
		CHECK_ERRNO(sock = socket(AF_UNIX, SOCK_STREAM, PF_UNSPEC));
		flags = fcntl(sock, F_GETFL, 0);
		if (flags < 0) {
			printf("fcntl() error");
			exit(EXIT_FAILURE);
		}
		CHECK_ERRNO(fcntl(sock, F_SETFL, flags | O_NONBLOCK));
	}
	else {
		int flags = nonblocking ? SOCK_NONBLOCK : 0;
		CHECK_ERRNO(sock = socket(AF_UNIX, SOCK_STREAM | flags, PF_UNSPEC));
	}
	return sock;
}

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
	  if (client_noop) {
		  printf("client no-op\n");
		  exit(EXIT_SUCCESS);
	  }

	  datasock = create_socket(nonblocking_client);

	  while (true) {
		  k = connect(datasock, (const struct sockaddr *)&addr, sizeof(struct sockaddr_un));
		  if (k < 0 && errno == EINPROGRESS) {
			  printf("connect() didn't block!\n");
			  break;
		  }
		  else if (k == 0) {
			  printf("connect() connected!\n");
			  break;
		  }
		  else if (k < 0) {
			  printf("connect() error: %s\n", strerror(errno));
			  sleep(1);
			  printf("connect() retrying\n");
		  }
	  }

	  if (nonblocking_client) {
		  int timeout_msecs = 500;
		  struct pollfd pfd[1];

		  pfd[0].fd = datasock;
		  pfd[0].events = POLLOUT;

		  printf("client is now polling\n");
		  while (true) {
			  CHECK_ERRNO(poll(pfd, 1, timeout_msecs));
			  if (pfd[0].revents & POLLOUT) {
				  printf("client POLLOUT: socket ready to write\n");
				  char *msg = "i am a message";
				  CHECK_ERRNO(send(datasock, msg, BUF_SIZE, 0));
				  printf("client sent message\n");
				  break;
			  }
			  else if (pfd[0].revents & POLLHUP) {
				  printf("client POLLHUP\n");
			  }
			  else {
				  printf("client poll timeouted\n");
			  }
		  }
	  }
	  else {
		  char *msg = "i am a message";
		  CHECK_ERRNO(send(datasock, msg, BUF_SIZE, 0));
		  printf("client sent message\n");
	  }
	  printf("client exited\n");

	  // FIXME graceful exit
  } else {
	  sock = create_socket(nonblocking_server);

	  CHECK_ERRNO(bind(sock, (const struct sockaddr *)&addr, sizeof(struct sockaddr_un)));
	  CHECK_ERRNO(listen(sock, 0));

	  do {
		  if (!nonblocking_server) {
			  datasock = accept(sock, NULL, NULL);

			  if (datasock <= 0) {
				  printf("accept() didn't block and returned: %s\n", strerror(errno));
				  break;
			  }
			  printf("socket ready to read/write\n");
			  CHECK_ERRNO(read(datasock, buf, BUF_SIZE));
			  buf[BUF_SIZE - 1] = 0;
			  printf("read(): got '%s'\n", buf);
		  }
		  else {
			  int timeout_msecs = 500;
			  struct pollfd pfd[2];

			  int n = 1;

			  pfd[0].fd = sock;
			  pfd[0].events = POLLIN;

			  printf("server is now polling\n");
			  while (true) {
				  CHECK_ERRNO(poll(pfd, n, timeout_msecs));

				  if (pfd[0].revents & POLLIN) {
					  printf("server POLLIN on 0\n");
					  CHECK_ERRNO(datasock = accept(sock, NULL, NULL));
					  if (datasock > 0) {
						  printf("server has accepted connection!\n");
						  pfd[1].fd = datasock;
						  pfd[1].events = POLLIN;
						  n++;
					  }
					  else {
						  printf("server failed to accept()\n");
					  }
				  }
				  else if (pfd[0].revents & POLLHUP) {
					  printf("server POLLHUP\n");
				  }
				  else if (n == 2 && (pfd[1].revents & POLLIN)) {
					  printf("server POLLIN on 1\n");
					  CHECK_ERRNO(read(pfd[1].fd, buf, BUF_SIZE));
					  buf[BUF_SIZE - 1] = 0;
					  printf("read(): got '%s'\n", buf);
					  break;
				  }
				  else {
					  printf("server poll timeouted\n");
				  }
			  }
		  }
		  printf("server exited\n");
	  } while (0);

	  wait(NULL);
  }

	return 0;
}
