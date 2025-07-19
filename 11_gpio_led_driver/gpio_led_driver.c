/*
 * gpio_led_driver.c - A Linux kernel module to manage a GPIO pin for LED on/off (Raspberry Pi 4 - BCM2711)
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/io.h>
#include <linux/slab.h>

#define DRIVER_NAME "gpio_led_driver"
#define PROC_NAME "gpio_led_driver"
#define DEVICE_LED_DRIVER_MAX_SIZE 1024
#define BCM2711_GPIO_ADDRESS 0xFE200000
#define BCM2711_GPIO_SIZE PAGE_SIZE

static struct proc_dir_entry *device_led_driver_proc = NULL;
static char data_buffer[DEVICE_LED_DRIVER_MAX_SIZE];
static void __iomem *gpio_registers = NULL;

static void gpio_pin_on(unsigned int pin)
{
    unsigned int fsel_index = pin / 10;
    unsigned int fsel_bitpos = pin % 10;

    void __iomem *gpio_fsel = gpio_registers + (fsel_index * 4);
    void __iomem *gpio_set = gpio_registers + 0x1C;

    u32 val = readl(gpio_fsel);
    val &= ~(7 << (fsel_bitpos * 3));         // Clear function bits
    val |=  (1 << (fsel_bitpos * 3));         // Set to output (001)
    writel(val, gpio_fsel);

    writel((1 << pin), gpio_set);             // Set pin high
}

static void gpio_pin_off(unsigned int pin)
{
    void __iomem *gpio_clr = gpio_registers + 0x28;
    writel((1 << pin), gpio_clr);             // Set pin low
}

static ssize_t device_led_driver_read(struct file *filep, char __user *buffer, size_t len, loff_t *offset)
{
    const char *msg = "Send pin,value to control GPIO (e.g., '17,1')\n";
    size_t msg_len = strlen(msg);

    if (*offset >= msg_len)
        return 0;

    if (len > msg_len - *offset)
        len = msg_len - *offset;

    if (copy_to_user(buffer, msg + *offset, len))
        return -EFAULT;

    *offset += len;
    return len;
}

static ssize_t device_led_driver_write(struct file *filep, const char __user *buffer, size_t size, loff_t *offset)
{
    unsigned int pin, value;

    if (size > DEVICE_LED_DRIVER_MAX_SIZE)
        size = DEVICE_LED_DRIVER_MAX_SIZE;

    memset(data_buffer, 0, sizeof(data_buffer));
    if (copy_from_user(data_buffer, buffer, size))
        return -EFAULT;

    printk(KERN_INFO DRIVER_NAME ": Received '%s'\n", data_buffer);

    if (sscanf(data_buffer, "%u,%u", &pin, &value) != 2) {
        printk(KERN_ERR DRIVER_NAME ": Invalid input format. Use 'pin,value'\n");
        return size;
    }

    if (pin > 27) {
        printk(KERN_ERR DRIVER_NAME ": Invalid GPIO pin (0-27 allowed)\n");
        return size;
    }

    if (value == 1) {
        gpio_pin_on(pin);
        printk(KERN_INFO DRIVER_NAME ": GPIO %u turned ON\n", pin);
    } else if (value == 0) {
        gpio_pin_off(pin);
        printk(KERN_INFO DRIVER_NAME ": GPIO %u turned OFF\n", pin);
    } else {
        printk(KERN_ERR DRIVER_NAME ": Invalid value (only 0 or 1 allowed)\n");
    }

    return size;
}

static const struct proc_ops device_led_driver_fops = {
    .proc_read  = device_led_driver_read,
    .proc_write = device_led_driver_write,
};

static int __init gpio_driver_init(void)
{
    printk(KERN_INFO DRIVER_NAME ": Initializing...\n");

    gpio_registers = ioremap(BCM2711_GPIO_ADDRESS, BCM2711_GPIO_SIZE);
    if (!gpio_registers) {
        printk(KERN_ERR DRIVER_NAME ": Failed to ioremap GPIO address\n");
        return -ENOMEM;
    }

    device_led_driver_proc = proc_create(PROC_NAME, 0666, NULL, &device_led_driver_fops);
    if (!device_led_driver_proc) {
        iounmap(gpio_registers);
        printk(KERN_ERR DRIVER_NAME ": Failed to create /proc entry\n");
        return -ENOMEM;
    }

    printk(KERN_INFO DRIVER_NAME ": Module loaded successfully\n");
    return 0;
}

static void __exit gpio_driver_exit(void)
{
    if (device_led_driver_proc)
        remove_proc_entry(PROC_NAME, NULL);

    if (gpio_registers)
        iounmap(gpio_registers);

    printk(KERN_INFO DRIVER_NAME ": Module unloaded\n");
}

module_init(gpio_driver_init);
module_exit(gpio_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Houssem Jarray");
MODULE_DESCRIPTION("A module to manage GPIO for LEDs on Raspberry Pi 4");
MODULE_VERSION("1.0");
