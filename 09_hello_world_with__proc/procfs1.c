/*
 * procf1.c - Using proc file system to create a device driver
 */

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/version.h>
#include <linux/string.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Houssem Jarray");
MODULE_DESCRIPTION("Simple /proc file example");
MODULE_VERSION("1.0");

#define PROCFS_NAME "helloworld"

static struct proc_dir_entry *our_proc_file;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 6, 0)
#define HAVE_PROC_OPS
#endif

static ssize_t procfile_read(struct file *file, char __user *buffer, size_t len, loff_t *offset)
{
    char msg[] = "HelloWorld!\n";
    size_t msg_len = strlen(msg);

    if (*offset >= msg_len)
        return 0; // EOF

    if (copy_to_user(buffer, msg, msg_len))
    {
        pr_err("copy_to_user failed\n");
        return -EFAULT;
    }

    *offset += msg_len;
    pr_info("procfile_read: %zu bytes sent\n", msg_len);
    return msg_len;
}

#ifdef HAVE_PROC_OPS
static const struct proc_ops proc_file_ops = {
    .proc_read = procfile_read,
};
#else
static const struct file_operations proc_file_ops = {
    .read = procfile_read,
};
#endif

static int __init procfs1_init(void)
{
    our_proc_file = proc_create(PROCFS_NAME, 0444, NULL, &proc_file_ops);
    if (!our_proc_file)
    {
        pr_alert("Failed to create /proc/%s\n", PROCFS_NAME);
        return -ENOMEM;
    }

    pr_info("/proc/%s created\n", PROCFS_NAME);
    return 0;
}

static void __exit procfs1_exit(void)
{
    proc_remove(our_proc_file);
    pr_info("/proc/%s removed\n", PROCFS_NAME);
}

module_init(procfs1_init);
module_exit(procfs1_exit);
