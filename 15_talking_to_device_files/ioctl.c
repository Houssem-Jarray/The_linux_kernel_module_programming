/*
 * ioctl.c - A simple example of ioctl usage in a Linux kernel module
 * This module registers a character device and handles IOCTLs for user-space communication.
 */

#include <linux/cdev.h>    // For character device structure and functions
#include <linux/fs.h>      // For file_operations structure
#include <linux/init.h>    // For __init and __exit macros
#include <linux/ioctl.h>   // For ioctl macros like _IOW, _IOR
#include <linux/module.h>  // For all kernel modules
#include <linux/slab.h>    // For kmalloc and kfree
#include <linux/uaccess.h> // For copy_to_user and copy_from_user
#include <linux/version.h> // For kernel version checks
#include <linux/device.h>  // Add this include for device class functions


// Structure to represent an ioctl argument
struct ioctl_arg
{
    unsigned int val;
};

// Define ioctl magic number (unique identifier for our ioctl group)
#define IOC_MAGIC '\x66' // Avoid trailing semicolon in macro definition

// Define IOCTL command codes
#define IOCTL_VALSET _IOW(IOC_MAGIC, 0, struct ioctl_arg) // Set full struct from user
#define IOCTL_VALGET _IOR(IOC_MAGIC, 1, struct ioctl_arg) // Get full struct to user
#define IOCTL_VALGET_NUM _IOR(IOC_MAGIC, 2, int)          // Get simple int to user
#define IOCTL_VALSET_NUM _IOW(IOC_MAGIC, 3, int)          // Set simple int from user

#define IOCTL_VAL_MAXNR 3
#define DRIVER_NAME "ioctltest"

// Device and character driver related globals
static unsigned int test_ioctl_major = 0; // Major number for the device
static unsigned int num_of_dev = 1;       // Number of devices to register (we use 1)
static struct cdev test_ioctl_cdev;       // Character device structure
static int ioctl_num = 0;                 // Example variable to demonstrate IOCTL usage
static struct class *ioctl_class = NULL;
static struct device *ioctl_device = NULL;

// Per-file structure to hold state
struct test_ioctl_data
{
    unsigned char val; // Store a value for this file
    rwlock_t lock;     // Lock to protect concurrent access
};

// IOCTL handler
static long test_ioctl_ioctl(struct file *filep, unsigned int cmd, unsigned long arg)
{
    struct test_ioctl_data *ioctl_data = filep->private_data;
    int retval = 0;
    unsigned char val;
    struct ioctl_arg data;
    memset(&data, 0, sizeof(data));

    switch (cmd)
    {
    case IOCTL_VALSET: // Set a struct from user space
        if (copy_from_user(&data, (void __user *)arg, sizeof(data)))
        {
            retval = -EFAULT;
            goto done;
        }

        pr_alert("IOCTL set val: %x\n", data.val);

        write_lock(&ioctl_data->lock);
        ioctl_data->val = data.val;
        write_unlock(&ioctl_data->lock);
        break;

    case IOCTL_VALGET: // Return a struct to user space
        read_lock(&ioctl_data->lock);
        val = ioctl_data->val;
        read_unlock(&ioctl_data->lock);
        data.val = val;

        if (copy_to_user((void __user *)arg, &data, sizeof(data)))
        {
            retval = -EFAULT;
            goto done;
        }
        break;

    case IOCTL_VALGET_NUM: // Return a simple int to user space
        retval = __put_user(ioctl_num, (int __user *)arg);
        break;

    case IOCTL_VALSET_NUM: // Set a simple int from user space
        retval = __get_user(ioctl_num, (int __user *)arg);
        break;

    default: // Invalid ioctl command
        retval = -ENOTTY;
    }

done:
    return retval;
}

// Read method — sends `val` repeatedly to user
static ssize_t test_ioctl_read(struct file *filep, char __user *buffer, size_t count, loff_t *offset)
{
    struct test_ioctl_data *ioctl_data = filep->private_data;
    unsigned char val;
    int retval;
    size_t i;

    // Read the internal value safely
    read_lock(&ioctl_data->lock);
    val = ioctl_data->val;
    read_unlock(&ioctl_data->lock);

    // Fill the user buffer with `val` repeated `count` times
    for (i = 0; i < count; i++)
    {
        if (copy_to_user(&buffer[i], &val, 1))
        {
            retval = -EFAULT;
            goto out;
        }
    }

    retval = count; // Return number of bytes read

out:
    return retval;
}

// Open method — called on `open()`
static int test_ioctl_open(struct inode *inode, struct file *filep)
{
    struct test_ioctl_data *ioctl_data;

    pr_alert("%s called\n", __func__);

    // Allocate memory for the file-specific data
    ioctl_data = kmalloc(sizeof(struct test_ioctl_data), GFP_KERNEL);
    if (!ioctl_data)
        return -ENOMEM;

    // Initialize the lock and default value
    rwlock_init(&ioctl_data->lock);
    ioctl_data->val = 0xFF;

    // Store pointer to our data in file->private_data
    filep->private_data = ioctl_data;

    return 0;
}

// Close method — called on `close()`
static int test_ioctl_close(struct inode *inode, struct file *filep)
{
    pr_alert("%s called\n", __func__);

    if (filep->private_data)
    {
        kfree(filep->private_data); // Free per-file memory
        filep->private_data = NULL;
    }

    return 0;
}

// File operations structure (defines behavior of our device file)
static struct file_operations fops = {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 4, 0)
    .owner = THIS_MODULE, // Specify module ownership (for 6.4+)
#endif
    .open = test_ioctl_open,
    .release = test_ioctl_close,
    .read = test_ioctl_read,
    .unlocked_ioctl = test_ioctl_ioctl, // IOCTL handler
};

// Module init — executed when `insmod` is run
static int __init ioctl_init(void)
{
    dev_t dev;
    int alloc_ret;
    int cdev_ret;

    // Dynamically allocate a major number
    alloc_ret = alloc_chrdev_region(&dev, 0, num_of_dev, DRIVER_NAME);
    if (alloc_ret)
    {
        pr_err("Failed to allocate char dev region\n");
        return alloc_ret;
    }

    test_ioctl_major = MAJOR(dev); // Save the major number

    pr_info("ioctltest: registered with major number %d\n", test_ioctl_major);

    // Initialize and add the character device
    cdev_init(&test_ioctl_cdev, &fops);
    cdev_ret = cdev_add(&test_ioctl_cdev, dev, num_of_dev);
    if (cdev_ret)
    {
        pr_err("Failed to add cdev\n");
        unregister_chrdev_region(dev, num_of_dev);
        return cdev_ret;
    }

    // Create device class
    ioctl_class = class_create("ioctl_class");  
    if (IS_ERR(ioctl_class))
    {
        pr_err("Failed to create class\n");
        cdev_del(&test_ioctl_cdev);
        unregister_chrdev_region(dev, num_of_dev);
        return PTR_ERR(ioctl_class);
    }

    // Create device node /dev/ioctltest
    ioctl_device = device_create(ioctl_class, NULL, dev, NULL, DRIVER_NAME);
    if (IS_ERR(ioctl_device))
    {
        pr_err("Failed to create device\n");
        class_destroy(ioctl_class);
        cdev_del(&test_ioctl_cdev);
        unregister_chrdev_region(dev, num_of_dev);
        return PTR_ERR(ioctl_device);
    }

    pr_info("%s driver registered successfully.\n", DRIVER_NAME);
    return 0;
}

// Module exit — executed when `rmmod` is run
static void __exit ioctl_exit(void)
{
    dev_t dev = MKDEV(test_ioctl_major, 0);

    device_destroy(ioctl_class, dev);
    class_unregister(ioctl_class);
    class_destroy(ioctl_class);

    cdev_del(&test_ioctl_cdev);
    unregister_chrdev_region(dev, num_of_dev);

    pr_alert("%s driver removed.\n", DRIVER_NAME);
}


// Register init and exit functions
module_init(ioctl_init);
module_exit(ioctl_exit);

// Module metadata
MODULE_LICENSE("GPL");
MODULE_AUTHOR("HOUSSEM JARRAY");
MODULE_DESCRIPTION("Kernel module example to communicate via ioctl");
MODULE_VERSION("1.0");
