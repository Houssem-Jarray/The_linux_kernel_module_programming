/*
* stop.c - Illustation of multi filed modules
*/

#include <linux/kernel.h> /* We are doing kernel work */
#include <linux/module.h> /* Specifically, a module */

void cleanup_module(void) {
    pr_info("Short is life of the kernel module\n");
}

MODULE_LICENSE("GPL");