/*
 * procfs2.c - using the /proc file for write and read
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/version.h>

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 6, 0)
#define HAVE_PROC_OPS
#endif

#define PROCFS_NAME "buffer1k"
#define PROCFS_MAX_SIZE 1024

/* This structure hold informations about the /proc file */
static struct proc_dir_entry *our_proc_file;

/* This buffer used to store characters for this modules */
static char procfs_buffer[PROCFS_MAX_SIZE];

/* The size of the buffer */
static unsigned long procfs_buffer_size = 0;

/*This function is called then the /proc file is read */
static ssize_t procfile_read(struct file *filep, char __user *buffer, size_t length, loff_t *offset)
{
    char s[13] = "HelloWorld\n";
    int len = strlen(s);

    if (*offset > 0)
    {
        return 0; // EOF
    }

    if (copy_to_user(buffer, s, len))
    {
        pr_info("copy to user failed\n");
        return -EFAULT;
    }
    *offset = len;
    pr_info("procfile read %s\n", s);
    return len;
}

/* This is function is called with the /proc file is written */
static ssize_t procfile_write(struct file *file, const char __user *buffer, size_t len, loff_t *off)
{
    if (len >= PROCFS_MAX_SIZE)
        procfs_buffer_size = PROCFS_MAX_SIZE - 1;
    else
        procfs_buffer_size = len;

    if (copy_from_user(procfs_buffer, buffer, procfs_buffer_size))
        return -EFAULT;

    procfs_buffer[procfs_buffer_size] = '\0';
    pr_info("procfile write: %s\n", procfs_buffer);

    return procfs_buffer_size;
}

#ifdef HAVE_PROC_OPS
static const struct proc_ops proc_file_fops = {
    .proc_read = procfile_read,
    .proc_write = procfile_write,
};
#else
static const struct file_operations proc_file_fops = {
    .read = procfile_read,
    .write = procfile_write,
}
#endif

static int __init procfs2_init(void)
{
    our_proc_file = proc_create(PROCFS_NAME, 0644, NULL, &proc_file_fops);
    if (NULL == our_proc_file)
    {
        pr_alert("Error: Could not initialize /proc/%s\n", PROCFS_NAME);
        return -ENOMEM;
    }
    pr_info("/proc/%s created\n", PROCFS_NAME);
    return 0;
}

static void __exit procfs2_exit(void)
{
    proc_remove(our_proc_file);
    pr_info("/proc/%s : removed\n", PROCFS_NAME);
    return;
}
module_init(procfs2_init);
module_exit(procfs2_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("HOUSSEM JARRAY");
