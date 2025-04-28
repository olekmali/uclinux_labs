/*
 *  module2.h - the header file with the ioctl definitions.
 *
 *  The declarations here have to be in a header file, because
 *  they need to be known both to the kernel module
 *  (in module2.c) and the process calling ioctl (ioctl.c)
 *
 *  Copyright © 2001, 2007 Peter Jay Salzman
 *  http://tldp.org/LDP/lkmpg/2.6/html/lkmpg.html
 */

#ifndef MODULE2_H
#define MODULE2_H

#include <linux/ioctl.h>

/* 
 * The major device number. If you are not sure whether 
 * the major number is already taken, use 0 and inspect
 * the log file. 
 * WARNIG: The module2.c code does not use 123,
 *         Inspect the log to see the actual number
 */
#define MAGIC_NUM 123

/* 
 * Set the message of the device driver 
 */
#define IOCTL_SET_MSG _IOR(MAGIC_NUM, 0, char *)
/*
 * _IOR means that we're creating an ioctl command 
 * number for passing information from a user process
 * to the kernel module. 
 *
 * The first arguments, MAGIC_NUM, is the major device 
 * number we're using.
 *
 * The second argument is the number of the command 
 * (there could be several with different meanings).
 *
 * The third argument is the type we want to get from 
 * the process to the kernel.
 */

/* 
 * Get the message of the device driver 
 */
#define IOCTL_GET_MSG _IOR(MAGIC_NUM, 1, char *)
/* 
 * This IOCTL is used for output, to get the message 
 * of the device driver. However, we still need the 
 * buffer to place the message in to be input, 
 * as it is allocated by the process.
 */

/* 
 * Get the n'th byte of the message 
 */
#define IOCTL_GET_NTH_BYTE _IOWR(MAGIC_NUM, 2, int)
/* 
 * The IOCTL is used for both input and output. It 
 * receives from the user a number, n, and returns 
 * Message[n]. 
 */

/* 
 * The name of the device file 
 */
#define DEVICE_FILE_NAME "/dev/mydevice"

#endif
