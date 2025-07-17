/*
 * hello-2.c - Illustrating the __init ,__initmacros, ansd_ __exit macros
 *              in a kernel module.
 */

#include <linux/module.h> /* Needed by all modules */
#include <linux/init.h>   /* Nedded for the macros __init , initdata and __exit */
#include <linux/printk.h> /* Nedded for pr_info() */

static int hello3_data __initdata = 3; /* Example of initdata */
static int __init hello_3_init(void)
{
    pr_info("Hello, world 3.\n");
    return 0;
}

static void __exit hello_3_init(void)
{
    pr_info("Goodbye, world 3.\n");
}

module_init(hello_3_init);
module_exit(hello_3_init);


// Specify the license type for the kernel module
MODULE_LICENSE("GPL");
