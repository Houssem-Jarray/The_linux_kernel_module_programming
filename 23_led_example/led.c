#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/io.h>
#include <linux/device.h>
#include <linux/cdev.h>

#define DEVICE_NAME "gpio_led"
#define DRIVER_NAME "gpio_led_driver"
#define GPIO_BASE_PHYS 0xFE200000 // BCM2711 GPIO base (Raspberry Pi 4)

#define GPFSEL_OFFSET 0x00
#define GPSET_OFFSET  0x1C
#define GPCLR_OFFSET  0x28
#define GPLEV_OFFSET  0x34

struct led
{
    int pin;
    int state; // 0 = off, 1 = on
    void (*on)(struct led *);
    void (*off)(struct led *);
};

struct LED_dev
{
    dev_t dev_num;
    struct cdev cdev;
    struct class *cls;
    struct device *dev;
};

static struct LED_dev leddev;
static void __iomem *gpio_base = NULL;
static struct led last_led = { .pin = -1, .state = 0 };

// ----------------- LED control ------------------
static void led_on(struct led *led)
{
    int reg = led->pin / 32;
    int shift = led->pin % 32;
    writel(1 << shift, gpio_base + GPSET_OFFSET + reg * 4);
    led->state = 1;
}

static void led_off(struct led *led)
{
    int reg = led->pin / 32;
    int shift = led->pin % 32;
    writel(1 << shift, gpio_base + GPCLR_OFFSET + reg * 4);
    led->state = 0;
}

static void set_pin_output(int pin)
{
    int reg = pin / 10;
    int shift = (pin % 10) * 3;
    u32 val = readl(gpio_base + GPFSEL_OFFSET + reg * 4);
    val &= ~(0b111 << shift);
    val |= (0b001 << shift); // output mode
    writel(val, gpio_base + GPFSEL_OFFSET + reg * 4);
}

// ----------------- File operations ----------------
static int device_open(struct inode *inode, struct file *filep)
{
    pr_info(DRIVER_NAME ": device opened\n");
    return 0;
}

static int device_release(struct inode *inode, struct file *filep)
{
    pr_info(DRIVER_NAME ": device closed\n");
    return 0;
}

static ssize_t device_write(struct file *file, const char __user *buf, size_t len, loff_t *offset)
{
    char input[16];
    int pin, state;

    if (len >= sizeof(input))
        return -EINVAL;

    if (copy_from_user(input, buf, len))
        return -EFAULT;

    input[len] = '\0';

    if (sscanf(input, "%d,%d", &pin, &state) != 2)
        return -EINVAL;

    if (pin < 0 || pin > 53)
        return -EINVAL;

    pr_info(DRIVER_NAME ": Received pin=%d state=%d\n", pin, state);

    set_pin_output(pin);

    last_led.pin = pin;
    last_led.on = led_on;
    last_led.off = led_off;

    if (state)
        last_led.on(&last_led);
    else
        last_led.off(&last_led);

    *offset += len;
    return len;
}

static ssize_t device_read(struct file *file, char __user *buf, size_t count, loff_t *offset)
{
    char result[32];
    int len;

    if (last_led.pin == -1)
        return 0; // Nothing to read yet

    // Read actual pin state from level register
    int reg = last_led.pin / 32;
    int shift = last_led.pin % 32;
    u32 level = readl(gpio_base + GPLEV_OFFSET + reg * 4);
    int state = (level & (1 << shift)) ? 1 : 0;

    len = snprintf(result, sizeof(result), "%d,%d\n", last_led.pin, state);

    if (*offset >= len)
        return 0;

    if (count > len - *offset)
        count = len - *offset;

    if (copy_to_user(buf, result + *offset, count))
        return -EFAULT;

    *offset += count;
    return count;
}

static struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = device_open,
    .write = device_write,
    .read = device_read,
    .release = device_release,
};

// ----------------- Init & Exit ------------------
static int __init led_init(void)
{
    int ret;

    gpio_base = ioremap(GPIO_BASE_PHYS, 0x100);
    if (!gpio_base)
    {
        pr_err(DRIVER_NAME ": ioremap failed\n");
        return -ENOMEM;
    }

    ret = alloc_chrdev_region(&leddev.dev_num, 0, 1, DEVICE_NAME);
    if (ret < 0)
    {
        iounmap(gpio_base);
        pr_err(DRIVER_NAME ": alloc_chrdev_region failed\n");
        return ret;
    }

    cdev_init(&leddev.cdev, &fops);
    ret = cdev_add(&leddev.cdev, leddev.dev_num, 1);
    if (ret < 0)
    {
        unregister_chrdev_region(leddev.dev_num, 1);
        iounmap(gpio_base);
        return ret;
    }

    leddev.cls = class_create(DEVICE_NAME);
    if (IS_ERR(leddev.cls))
    {
        cdev_del(&leddev.cdev);
        unregister_chrdev_region(leddev.dev_num, 1);
        iounmap(gpio_base);
        return PTR_ERR(leddev.cls);
    }

    leddev.dev = device_create(leddev.cls, NULL, leddev.dev_num, NULL, DEVICE_NAME);
    if (IS_ERR(leddev.dev))
    {
        class_destroy(leddev.cls);
        cdev_del(&leddev.cdev);
        unregister_chrdev_region(leddev.dev_num, 1);
        iounmap(gpio_base);
        return PTR_ERR(leddev.dev);
    }

    pr_info(DRIVER_NAME ": Module loaded. Device /dev/%s created\n", DEVICE_NAME);
    return 0;
}

static void __exit led_exit(void)
{
    device_destroy(leddev.cls, leddev.dev_num);
    class_destroy(leddev.cls);
    cdev_del(&leddev.cdev);
    unregister_chrdev_region(leddev.dev_num, 1);
    iounmap(gpio_base);
    pr_info(DRIVER_NAME ": Module unloaded\n");
}

module_init(led_init);
module_exit(led_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Houssem Jarray");
MODULE_DESCRIPTION("GPIO LED driver using struct led and direct register access");
