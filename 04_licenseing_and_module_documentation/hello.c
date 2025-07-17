/*
* hello.c - Demonstates module documentation and licensing in a kernel module.
*/

#include <linux/module.h> /* Needed by all modules */
#include <linux/init.h>   /* Needed for the macros __init and __exit */
#include <linux/printk.h> /* Needed for pr_info() */

/* Module Documentation and licensing */

MODULE_LICENSE("GPL");
MODULE_AUTHOR("HOUSSEM JARRAY");
MODULE_DESCRIPTION("A simple Hello World kernel module demonstrating licensing and documentation.");

static int __init hello_init(void) {
    pr_info("Hello, world from the licensing and documentation module.\n");
    return 0;
}

static void __exit hello_exit(void) {
    pr_info("Goodbye, world from the licensing and documentation module.\n");
}
module_init(hello_init);
module_exit(hello_exit);