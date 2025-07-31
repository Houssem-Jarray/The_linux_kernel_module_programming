/*
 * completion.c - Example of using completion in kernel module
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/completion.h>
#include <linux/init.h>
#include <linux/err.h> /* for IS_ERR() */
#include <linux/kthread.h>
#include <linux/printk.h>
#include <linux/version.h>

static struct completion crank_completion;
static struct completion flywheel_completion;

static int machine_crank_thread(void *arg)
{
    pr_info("Turn the crank\n");

    complete_all(&crank_completion);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 17, 0)
    kthread_complete_and_exit(&crank_completion, 0);
#else
    complete_and_exit(&crank_completion, 0);
#endif
}

static int machine_flywheel_spinup_thread(void *arg)
{
    pr_info("Turn the flywheel\n");
    // wait for the crank to complete before spinning up the flywheel
    wait_for_completion(&crank_completion);

    pr_info("Flywheel is spinning up\n");
    complete_all(&flywheel_completion);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 17, 0)
    kthread_complete_and_exit(&flywheel_completion, 0);
#else
    complete_and_exit(&flywheel_completion, 0);
#endif
}

static int __init completions_init(void)
{

    struct task_struct *crank_thread;
    struct task_struct *flywheel_thread;

    pr_info("completion examples");

    init_completion(&crank_completion);
    init_completion(&flywheel_completion);

    // create two kernel threads
    crank_thread = kthread_create(machine_crank_thread, NULL, "Kthread Crank");
    if (IS_ERR(crank_thread))
        goto ERROR_THREAD_1;
    flywheel_thread = kthread_create(machine_flywheel_spinup_thread, NULL, "Kthread Flywheel");
    if (IS_ERR(flywheel_thread))
        goto ERROR_THREAD_2;
    
    // wake up the threads
    pr_info("Waking up crank thread and flywheel thread");
    wake_up_process(flywheel_thread);
    wake_up_process(crank_thread);

    return 0;

ERROR_THREAD_2:
    kthread_stop(crank_thread);
ERROR_THREAD_1:
    return -1;
}

static void __exit completions_exit(void)
{
    wait_for_completion(&crank_completion);
    wait_for_completion(&flywheel_completion);

    pr_info("completions exit\n");
}
module_init(completions_init);
module_exit(completions_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("HOUSSEM JARRAY");
MODULE_DESCRIPTION("Example of using completion in kernel module");
MODULE_VERSION("1.0");