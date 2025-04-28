/*
 *  module2.c - Create an input/output character device
 *
 *  Copyright © 2001, 2007 Peter Jay Salzman
 *  http://tldp.org/LDP/lkmpg/2.6/html/lkmpg.html
 *
 *  Modified by Dr. Aleksander Malinowski 4/24/2013
 *    deprecated dependence on kernel lock/unlock was removed
 *  Modified by Dr. Aleksander Malinowski 4/3/2017
 *    if down_interruptible(...) lock acquisitionn is interrupted and returns nonzero
 *    then do not proceed, instead return error -ERESTARTSYS so that the upper layer may retry the call later
 *    not that any progress iinside the device driver must be undone before such return
 */

#include <linux/kernel.h>       /* We're doing kernel work */
#include <linux/module.h>       /* Specifically, a module */
// #include <linux/smp_lock.h>  /* access semaphore to lock and unlock the kernel - deprecated */
//   down_interruptible(&device_semaphore_xxx_lock);
//   up(&device_semaphore_xxx_lock);
//       separate semaphonre is used for read/write and for ioctl as ioctl calls read/write
//       and the semaphonre is not recurisve unlike the deprecated Big kernel Lock.
#include <linux/semaphore.h>    /* semaphore functinality */
#include <linux/fs.h>           /* device struct data type definitions */
#include <linux/uaccess.h>      /* for get_user and put_user */

#include "module2.h"

#define SUCCESS 0
#define DEVICE_NAME "module2"
#define BUF_LEN 80

/* 
 * we are using a dynamic major number, we need to save it so that we can clean up the module
 */
static int major_num;

/* 
 * semaphore to ensure correct work of the module
 * to be used instead of deprecated down_interruptible(&device_semaphore_lock)/up(&device_semaphore_lock)
 */
static struct semaphore device_semaphore_rw_lock;
static struct semaphore device_semaphore_ioctl_lock;

/* 
 * Is the device open right now? Used to prevent
 * concurent access into the same device 
 */
static int device_open_count = 0;

/* 
 * The message the device will give when asked 
 */
static char Message[BUF_LEN];

/* 
 * How far did the process reading the message get?
 * Useful if the message is larger than the size of the
 * buffer we get to fill in device_read. 
 */
static char *Message_Ptr;

/* 
 * This is called whenever a process attempts to open the device file 
 */
static int device_open(struct inode *inode, struct file *file)
{
    static int counter = 0; /* static variable is kept between function calls */

#ifdef DEBUG
    printk(KERN_INFO "device_open(%p)\n", file);
#endif

    /* 
     * We don't want to talk to two processes at the same time 
     */
    if (device_open_count)
        return -EBUSY;

    if (down_interruptible(&device_semaphore_rw_lock))
        return -ERESTARTSYS;
    device_open_count++;
    /*
     * Initialize the message 
     */
    sprintf(Message, "I already told you %d times Hello world!\n", counter++);
    Message_Ptr = Message;
    try_module_get(THIS_MODULE);
    up(&device_semaphore_rw_lock);

    return SUCCESS;
}

static int device_release(struct inode *inode, struct file *file)
{
#ifdef DEBUG
    printk(KERN_INFO "device_release(%p,%p)\n", inode, file);
#endif

    /* 
     * We're now ready for our next caller 
     */
    if (down_interruptible(&device_semaphore_rw_lock))
        return -ERESTARTSYS;
    device_open_count--;
    module_put(THIS_MODULE);
    up(&device_semaphore_rw_lock);

    return SUCCESS;
}

/* 
 * This function is called whenever a process which has already opened the
 * device file attempts to read from it.
 */
static ssize_t device_read(struct file *file, char __user * buffer, size_t length, loff_t * offset)
{
    /* 
     * Number of bytes actually written to the buffer 
     */
    int bytes_read = 0;

#ifdef DEBUG
    printk(KERN_INFO "device_read(%p,%p,%d)\n", file, buffer, length);
#endif

    /* 
     * If we're at the end of the message, return 0
     * (which signifies end of file) 
     */
    if (*Message_Ptr == 0)
        return 0;

    /* 
     * Actually put the data into the buffer 
     */
    if (down_interruptible(&device_semaphore_rw_lock))
        return -ERESTARTSYS;
    while (length && *Message_Ptr) {

        /* 
         * Because the buffer is in the user data segment,
         * not the kernel data segment, assignment wouldn't
         * work. Instead, we have to use put_user which
         * copies data from the kernel data segment to the
         * user data segment. 
         */
        put_user(*(Message_Ptr++), buffer++);
        length--;
        bytes_read++;
    }
    up(&device_semaphore_rw_lock);

#ifdef DEBUG
    printk(KERN_INFO "Read %d bytes, %d left\n", bytes_read, length);
#endif

    /* 
     * Read functions are supposed to return the number
     * of bytes actually inserted into the buffer 
     */
    return bytes_read;
}

/* 
 * This function is called when somebody tries to
 * write into our device file. 
 */
static ssize_t device_write(struct file *file, const char __user * buffer, size_t length, loff_t * offset)
{
    int i;

#ifdef DEBUG
    printk(KERN_INFO "device_write(%p,%s,%d)", file, buffer, length);
#endif

    if (down_interruptible(&device_semaphore_rw_lock))
        return -ERESTARTSYS;
    for (i = 0; i < length && i < BUF_LEN; i++)
        get_user(Message[i], buffer + i);
    Message_Ptr = Message;
    up(&device_semaphore_rw_lock);

    /* 
     * Again, return the number of input characters used 
     */
    return i;
}

/* 
 * This function is called when somebody tries to
 * change the current read/write position in a file, and
 * the new position is returned as a (positive) return value.
 */
static loff_t device_llseek (struct file *file, loff_t offset, int origin)
{
#ifdef DEBUG
    printk(KERN_INFO "device_llseek(%p,%ld,%d)", file, offset, origin);
#endif
    
    /* 
     * Again, the new position is returned as a (positive) return value
     * Errors are signaled by a negative return value
     */
    return 0;
}

/* 
 * This function is called whenever a process tries to do an ioctl on our
 * device file. We get two extra parameters (additional to the inode and file
 * structures, which all device functions get): the number of the ioctl called
 * and the parameter given to the ioctl function.
 *
 * If the ioctl is write or read/write (meaning output is returned to the
 * calling process), the ioctl call returns the output of this function.
 *
 */
static long device_ioctl( struct file *file, unsigned int ioctl_num, unsigned long ioctl_param)
{
    int i;
    char *temp;
    char ch;
    int status = SUCCESS;

    
    /* 
     * Switch according to the ioctl called 
     */
    switch (ioctl_num) {
    case IOCTL_SET_MSG:
        /* 
         * Receive a pointer to a message (in user space) and set that
         * to be the device's message.  Get the parameter given to 
         * ioctl by the process. 
         */
        temp = (char *)ioctl_param;

        /* 
         * Find the length of the message 
         */
        get_user(ch, temp);
        for (i = 0; ch && i < BUF_LEN; i++, temp++)
            get_user(ch, temp);

        device_write(file, (char *)ioctl_param, i, 0);
        break;

    case IOCTL_GET_MSG:
        /* 
         * Give the current message to the calling process - 
         * the parameter we got is a pointer, fill it. 
         */
        i = device_read(file, (char *)ioctl_param, 99, 0);

        /* 
         * Put a zero at the end of the buffer, so it will be 
         * properly terminated 
         */
        put_user('\0', (char *)ioctl_param + i);
        break;

    case IOCTL_GET_NTH_BYTE:
        /* 
         * This ioctl is both input (ioctl_param) and 
         * output (the return value of this function) 
         */
        return Message[ioctl_param];
        break;
    default:
        status = -EINVAL; /* invalid IOCTL argument */
    }

    return status;
}

/* Module Declarations */

/* 
 * This structure will hold the functions to be called
 * when a process does something to the device we
 * created. Since a pointer to this structure is kept in
 * the devices table, it can't be local to
 * init_module. NULL is for unimplemented functions. 
 */
struct file_operations fops = {
    .owner   = THIS_MODULE,
    .llseek  = device_llseek,
    .read    = device_read,
    .write   = device_write,
    .unlocked_ioctl = device_ioctl,
    .open    = device_open,
    .release = device_release,  /* a.k.a. close */
};

/* 
 * Initialize the module - Register the character device 
 */
int init_module()
{

    /* 
     * initialize the device driver semaphore
     */
    sema_init(&device_semaphore_rw_lock, 1);
    sema_init(&device_semaphore_ioctl_lock, 1);

    /* 
     * Register the character device (atleast try) 
     */
    major_num = register_chrdev(0, DEVICE_NAME, &fops); /* use this version for the first available major number */
    /* major_num = register_chrdev(MAJOR_NUM, DEVICE_NAME, &fops); -- use this version for preset major number */

    /* 
     * Negative values signify an error 
     */
    if (major_num < 0) {
        printk(KERN_ALERT "%s failed with %d\n",
            "Sorry, registering the character device ", major_num);
        return major_num;
    }

    printk(KERN_INFO "%s The major device number is %d.\n",
            "Registeration is a success", major_num);
    printk(KERN_INFO "If you want to talk to the device driver,\n");
    printk(KERN_INFO "you'll have to create a device file. \n");
    printk(KERN_INFO "We suggest you use:\n");
    printk(KERN_INFO "mknod %s c %d 0\n", DEVICE_FILE_NAME, major_num);
    printk(KERN_INFO "The device file name is important, because\n");
    printk(KERN_INFO "the ioctl test program assumes that's the file you'll use.\n");

    return 0;
}

/* 
 * Cleanup - unregister the appropriate file from /proc 
 */
void cleanup_module()
{
    /* 
     * Unregister the device 
     */
    unregister_chrdev(major_num, DEVICE_NAME);
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Aleksander Malinowski");
MODULE_DESCRIPTION("Example of a module with a semaphore");
