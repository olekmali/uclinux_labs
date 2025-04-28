/*  
 *  hello1.c - The simplest kernel module.
 *  Also illustrating the __init, __initdata and __exit macros.
 *
 *  Copyright © 2001 Peter Jay Salzman
 *  http://tldp.org/LDP/lkmpg/2.6/html/lkmpg.html
 */
#include <linux/module.h>   /* Needed by all modules */
#include <linux/kernel.h>   /* Needed for KERN_INFO */
#include <linux/init.h>     /* Needed for the macros */

#define DRIVER_AUTHOR "Peter Jay Salzman <p@dirac.org>"
#define DRIVER_DESC   "A sample driver"

static int hello1_data __initdata = 1;

static int __init hello1_init(void)
{
    printk(KERN_INFO "Hello, world from <insert-your-name-here>%d\n", hello1_data);
    return 0;
}

static void __exit hello1_exit(void)
{
    printk(KERN_INFO "Goodbye, world 1\n");
}

module_init(hello1_init);
module_exit(hello1_exit);

MODULE_LICENSE("GPL");                  /* What is the licensing for distribution? */
MODULE_AUTHOR(DRIVER_AUTHOR);           /* Who wrote this module? */
MODULE_DESCRIPTION(DRIVER_DESC);        /* What does this module do? */
