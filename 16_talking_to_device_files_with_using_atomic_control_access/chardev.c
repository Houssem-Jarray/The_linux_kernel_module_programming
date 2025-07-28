/*
 * chardev2.c - Create an input/output character device with dynamic buffer
 */

#include <linux/atomic.h>      // For atomic operations
#include <linux/cdev.h>        // For character device structure and functions
#include <linux/device.h>      // For device class functions
#include <linux/fs.h>          // For file_operations structure
#include <linux/init.h>        // For __init and __exit macros
#include <linux/kernel.h>      // For printk and pr_info
#include <linux/module.h>      // For all kernel modules
#include <linux/slab.h>        // For kmalloc and kfree
#include <linux/uaccess.h>     // For copy_to_user and copy_from_user
#include <linux/version.h>     // For kernel version checks

#include "chardev.h"

#define DEVICE_NAME "chardev"

enum
{
    CDEV_NOT_USED = 0,
    CDEV_EXCLUSIVE_OPEN = 1,
};

/* Atomic to prevent concurrent open */
static atomic_t already_open = ATOMIC_INIT(CDEV_NOT_USED);

/* Dynamically allocated buffer for message */
static char *message = NULL;
static size_t message_size = 0;

static struct class *cls = NULL;

/* Read from device */
static ssize_t device_read(struct file *file, char __user *buffer, size_t length, loff_t *offset)
{
    ssize_t bytes_read = 0;

    if (!message || *offset >= message_size)
        return 0; // EOF

    if (length > message_size - *offset)
        length = message_size - *offset;

    if (copy_to_user(buffer, message + *offset, length))
        return -EFAULT;

    *offset += length;
    bytes_read = length;

    pr_info("Read %zd bytes; %lld bytes left\n", bytes_read, message_size - *offset);

    return bytes_read;
}

/* Write to device */
static ssize_t device_write(struct file *file, const char __user *buffer, size_t length, loff_t *offset)
{
    char *new_msg;

    if (length == 0)
        return 0;

    /* Allocate/reallocate buffer to hold new message */
    new_msg = kmalloc(length + 1, GFP_KERNEL);
    if (!new_msg)
        return -ENOMEM;

    if (copy_from_user(new_msg, buffer, length)) {
        kfree(new_msg);
        return -EFAULT;
    }

    new_msg[length] = '\0'; // Null terminate

    /* Free old message buffer */
    kfree(message);
    message = new_msg;
    message_size = length;

    pr_info("Written %zu bytes to device\n", length);

    return length;
}

/* ioctl handler */
static long device_ioctl(struct file *file, unsigned int ioctl_num, unsigned long ioctl_param)
{
    long ret = 0;

    switch (ioctl_num)
    {
    case IOCTL_SET_MSG:
    {
        char __user *user_msg = (char __user *)ioctl_param;
        char *kbuf;
        size_t len;

        if (!user_msg)
            return -EINVAL;

        /* Let's limit the maximum message length */
        len = strnlen_user(user_msg, 1024);
        if (len == 0 || len > 1024)
            return -EINVAL;

        kbuf = kmalloc(len, GFP_KERNEL);
        if (!kbuf)
            return -ENOMEM;

        if (copy_from_user(kbuf, user_msg, len)) {
            kfree(kbuf);
            return -EFAULT;
        }

        /* Free old message */
        kfree(message);

        message = kbuf;
        message_size = len - 1; // exclude terminating null from strnlen_user

        pr_info("IOCTL: Set message of size %zu\n", message_size);

        break;
    }
    case IOCTL_GET_MSG:
    {
        /* ioctl_param is a user pointer to buffer where we copy message */
        char __user *user_buf = (char __user *)ioctl_param;

        if (!user_buf)
            return -EINVAL;

        if (message == NULL)
            return -ENODATA;

        if (copy_to_user(user_buf, message, message_size + 1))
            return -EFAULT;

        pr_info("IOCTL: Get message\n");
        break;
    }
    case IOCTL_GET_NTH_BYTE:
        if (ioctl_param >= message_size)
            ret = -EINVAL;
        else if (message)
            ret = (long)message[ioctl_param];
        else
            ret = -ENODATA;
        break;
    default:
        ret = -ENOTTY;
        break;
    }

    return ret;
}

/* Device open */
static int device_open(struct inode *inode, struct file *file)
{
    if (atomic_cmpxchg(&already_open, CDEV_NOT_USED, CDEV_EXCLUSIVE_OPEN))
        return -EBUSY;

    try_module_get(THIS_MODULE);
    pr_info("device_open()\n");
    return 0;
}

/* Device release */
static int device_release(struct inode *inode, struct file *file)
{
    atomic_set(&already_open, CDEV_NOT_USED);
    module_put(THIS_MODULE);
    pr_info("device_release()\n");
    return 0;
}

static const struct file_operations fops = {
    .owner = THIS_MODULE,
    .read = device_read,
    .write = device_write,
    .unlocked_ioctl = device_ioctl,
    .open = device_open,
    .release = device_release,
};

static int major_num;

/* Module init */
static int __init chardev_init(void)
{
    major_num = register_chrdev(0, DEVICE_NAME, &fops);
    if (major_num < 0) {
        pr_err("Failed to register character device\n");
        return major_num;
    }

    #if LINUX_VERSION_CODE >= KERNEL_VERSION(6,4,0)
    cls = class_create(THIS_MODULE);
    #else
    cls = class_create(THIS_MODULE, DEVICE_NAME);
    #endif
    if (IS_ERR(cls)) {
        unregister_chrdev(major_num, DEVICE_NAME);
        pr_err("Failed to create device class\n");
        return PTR_ERR(cls);
    }

    if (device_create(cls, NULL, MKDEV(major_num, 0), NULL, DEVICE_NAME) == NULL) {
        class_destroy(cls);
        unregister_chrdev(major_num, DEVICE_NAME);
        pr_err("Failed to create device\n");
        return -ENOMEM;
    }

    pr_info("Device registered with major number %d\n", major_num);

    return 0;
}

/* Module exit */
static void __exit chardev_exit(void)
{
    device_destroy(cls, MKDEV(major_num, 0));
    class_destroy(cls);
    unregister_chrdev(major_num, DEVICE_NAME);

    /* Free allocated message */
    kfree(message);

    pr_info("Device unregistered\n");
}

module_init(chardev_init);
module_exit(chardev_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("HOUSSEM JARRAY");
MODULE_DESCRIPTION("Kernel module example: character device with dynamic buffer and ioctl");
MODULE_VERSION("1.1");
