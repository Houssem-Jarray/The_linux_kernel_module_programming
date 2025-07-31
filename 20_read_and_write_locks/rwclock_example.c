/*
* read_write_example.c - A simple read/write lock example for kernel module
*/

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/rwlock.h>

static DEFINE_RWLOCK(myrwlock);
static void example_read_lock(void) {
    unsigned long flags;

    read_lock_irqsave(&myrwlock, flags);
    printk(KERN_INFO "Read lock acquired\n");

    /* READ FROM SOMETHING */
    read_unlock_irqrestore(&myrwlock, flags);
    printk(KERN_INFO "Read lock released\n");
}
static void example_write_lock(void) {
    unsigned long flags;

    write_lock_irqsave(&myrwlock, flags);
    printk(KERN_INFO "Write lock acquired\n");

    /* WRITE TO SOMETHING */
    write_unlock_irqrestore(&myrwlock, flags);
    printk(KERN_INFO "Write lock released\n");
}

static int __init rwlock_example_init(void) {
    printk(KERN_INFO "Read/Write Lock Module Initialized\n");
    example_read_lock();
    example_write_lock();

    return 0;
}

static void __exit rwlock_example_exit(void) {
    printk(KERN_INFO "Read/Write Lock Module Exited\n");
}

module_init(rwlock_example_init);
module_exit(rwlock_example_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("HOUSSEM JARRAY");
MODULE_DESCRIPTION("A simple example of read/write locks in a kernel module");  
MODULE_VERSION("1.0");