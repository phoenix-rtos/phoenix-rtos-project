/*
 * Phoenix-RTOS
 *
 * IPC benchmark - server component
 *
 * Copyright 2026 Phoenix Systems
 * Author: Adam Greloch
 *
 * This file is part of Phoenix-RTOS.
 *
 * %LICENSE%
 */

#ifndef _IPCBENCH_SERVER_H_
#define _IPCBENCH_SERVER_H_

#include <stdint.h>
#include <sys/types.h>


/* Spawn an echo server that registers at dev_path.
 * If use_respond_and_recv is set, uses msgRespondAndRecv instead of
 * separate msgRecv + msgRespond. Returns child PID. */
pid_t server_spawn_echo(const char *dev_path, int use_respond_and_recv, int prio);


/* Spawn a forwarding server that registers at dev_path and forwards
 * messages to backend_port. Returns child PID. */
pid_t server_spawn_forwarder(const char *dev_path, uint32_t backend_port);


/* Spawn a POSIX-style server (handles mtOpen/mtClose/mtRead/mtWrite/
 * mtGetAttr/mtGetAttrAll/mtLookup/mtCreate/mtLink/mtUnlink) at dev_path.
 * Manages a simple in-memory file tree. Returns child PID. */
pid_t server_spawn_posix(const char *dev_path);


/* Kill a previously spawned server */
void server_kill(pid_t pid);


#endif /* _IPCBENCH_SERVER_H_ */
