/*
 * Phoenix-RTOS
 *
 * Operating system kernel
 *
 * Error definitions
 *
 * Copyright 2012-2015, 2016-2017 Phoenix Systems
 * Author: Pawel Pisarczyk, Pawel Kolodziej, Jacek Popko
 *
 * This file is part of Phoenix-RTOS.
 *
 * %LICENSE%
 */

#ifndef _PHOENIX_ERRNO_H_
#define _PHOENIX_ERRNO_H_


#define  EOK           0  /* No error */
#define  EPERM         1  /* Operation not permitted */
#define  EINTR         4  /* Interrupted system call */
#define  ENOMEM       12  /* Out of memory */
#define  EFAULT       14  /* Bad address */
#define  EBUSY        16  /* Device or resource busy */
#define  EEXIST       17  /* File exists */
#define  EINVAL       22  /* Invalid argument */
#define  ENOTTY       25  /* Not a typewriter */
#define  ETIME        62  /* Timer expired */


#endif
