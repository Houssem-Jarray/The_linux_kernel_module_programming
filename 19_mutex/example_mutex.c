/*
 * example_mutex.c - A simple mutex example for kernel module
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/mutex.h>
#include <linux/init.h>

static DEFINE_MUTEX(mymutex);

static int __init example_mutex_init(void)
{
    int ret;

    printk(KERN_INFO "Example Mutex Module Initialized\n");

    ret = mutex_trylock(&mymutex);
    if (ret != 0)
    {
        pr_info("The mutex failed to lock!\n");
        if (mutex_is_locked(&mymutex) == 0)
            pr_info("The mutex failed to lock!\n");
        mutex_unlock(&mymutex);
        pr_info("The mutex is now unlocked.\n");
    }
    else
    {
        pr_info("Failed to lock!\n");
    }
    return 0;
}

static void __exit example_mutex_exit(void)
{
    printk(KERN_INFO "Example Mutex Module Exited\n");
}

module_init(example_mutex_init);
module_exit(example_mutex_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("HOUSSEM JARRAY");
MODULE_DESCRIPTION("A simple example of using mutex in a kernel module");
MODULE_VERSION("1.0");