/*
 * procfs3.c - An example of creating a kernel device driver with manage the
 * proc file system (procfs) with standard filesystem
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/sched.h>
#include <linux/uaccess.h>
#include <linux/version.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0)
#include <linux/minmax.h>
#endif
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 6, 0)
#define HAVE_PROC_OPS
#endif

#define PROC_MAX_SIZE 2048UL
#define PROCFS_ENTRY_FILENAME "buffer2k"

static struct proc_dir_entry *our_proc_file = NULL;
static char procfs_buffer[PROC_MAX_SIZE]; // le buffer partagé
static unsigned long procfs_buffer_size = 0; // taille actuelle des donneés

// lire que une seule fois
static ssize_t procfs_read(struct file *filep, char __user *buffer, size_t buffer_length, loff_t *offset)
{
    if (*offset || procfs_buffer_size == 0)
    {
        pr_debug("procfs_read: END\n");
        *offset = 0;
        return 0;
    }
    procfs_buffer_size = min(procfs_buffer_size, buffer_length);
    if (copy_to_user(buffer, procfs_buffer, procfs_buffer_size))
    {
        return -EFAULT;
    }
    *offset += procfs_buffer_size;

    pr_debug("procfs_read: read %lu bytes\n", procfs_buffer_size);
    return procfs_buffer_size;
}

static ssize_t procfs_write(struct file *filep, const char __user *buffer, size_t buffer_length, loff_t *offset)
{
    procfs_buffer_size = min(PROC_MAX_SIZE, buffer_length);
    if (copy_from_user(procfs_buffer, buffer, procfs_buffer_size))
        return -EFAULT;
    *offset += procfs_buffer_size;
    
    pr_debug("procfs_write: write %lu bytes\n", procfs_buffer_size);
    return procfs_buffer_size;
}

static int procfs_open(struct inode *inode, struct file *filep)
{
    try_module_get(THIS_MODULE);
    return 0;
}
static int procfs_release(struct inode *inode, struct file *filep)
{
    module_put(THIS_MODULE);
    return 0;
}

#ifdef HAVE_PROC_OPS
static struct proc_ops file_ops_4_our_proc_file = {
    .proc_read = procfs_read,
    .proc_write = procfs_write,
    .proc_open = procfs_open,
    .proc_release = procfs_release,
};
#else
static struct file_operations file_ops_4_our_proc_file = {
    .read = procfs_read,
    .write = procfs_write,
    .open = procfs_open,
    .release = procfs_release,
};
#endif

static int __init procfs3_init(void)
{
    pr_info("Loading the module procfs3\n");

    our_proc_file = proc_create(PROCFS_ENTRY_FILENAME, 0644, NULL, &file_ops_4_our_proc_file);
    if (!our_proc_file) {
        pr_err("procfs3: Failed to create /proc/%s\n", PROCFS_ENTRY_FILENAME);
        return -ENOMEM;
    }

    return 0;
}

static void __exit procfs3_exit(void)
{
    pr_info("Exiting the procfs3 module\n");
    proc_remove(our_proc_file);
}

module_init(procfs3_init);
module_exit(procfs3_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("HOUSSEM JARRAY");
MODULE_DESCRIPTION("A module proc that use the standard filesystem");
MODULE_VERSION("1.0");