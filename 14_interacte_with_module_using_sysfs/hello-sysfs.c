/*
 * hello-sysfs.c - interacte with my kernel module using sysfs
 */

#include <linux/kernel.h> /* We are doing kernel work */
#include <linux/module.h> /* Specifically, a modules */
#include <linux/init.h>
#include <linux/kobject.h>
#include <linux/string.h>
#include <linux/sysfs.h>

/* Meta Informations */
MODULE_LICENSE("GPL");
MODULE_AUTHOR("HOUSSEM JARRAY");
MODULE_DESCRIPTION("Create a module to present the interact of sysfs with our module");
MODULE_VERSION("1.0");

static struct kobject *mymodule;

static int myvariable = 0;

static ssize_t myvariable_show(struct kobject *kobj, struct kobj_attribute *attr,
                               char *buf)
{
    return sprintf(buf, "%d\n", myvariable);
}
static ssize_t myvariable_store(struct kobject *kobj, struct kobj_attribute *attr,
                                const char *buf, size_t count)
{
    sscanf(buf, "%d", &myvariable);
    return count;
}
static struct kobj_attribute myvariable_attribute =
    __ATTR(myvariable, 0660, myvariable_show, myvariable_store);

static int __init hellosysfs_init(void)
{
    int error = 0;
    pr_info("mymodule: initialized\n");
    mymodule = kobject_create_and_add("mymodule", kernel_kobj);
    if (!mymodule)
        return -ENOMEM;

    error = sysfs_create_file(mymodule, &myvariable_attribute.attr);
    if(error) {
        kobject_put(mymodule);
        pr_info("failed to create myvariable file int /sys/kernel/mymodule\n");
    }
    return error;
}
static void __exit hellosysfs_exit(void)
{
    pr_info("My module: Exit success\n");
    kobject_put(mymodule);
}
module_init(hellosysfs_init);
module_exit(hellosysfs_exit);
