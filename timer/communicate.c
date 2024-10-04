#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/kobject.h>
#include <linux/sysfs.h>
#include <linux/uaccess.h>

// Define the kobject
static struct kobject *communicate_kobj;

// Function to show the value
static ssize_t stop_timer_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf) {
    return sprintf(buf, "Stop timer command.\n");
}

// Function to store the value
static ssize_t stop_timer_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count) {
    if (strncmp(buf, "stop", 4) == 0) {
        // Implement your timer stop functionality here
        printk(KERN_INFO "Timer stopped\n");
    }
    return count;
}

// Define the kobj_attribute
static struct kobj_attribute stop_timer_attribute = __ATTR(stop_timer, 0664, stop_timer_show, stop_timer_store);

// Initialization function
static int __init communicate_init(void) {
    int error;

    // Create the kobject in the kernel namespace
    communicate_kobj = kobject_create_and_add("communicate", kernel_kobj);
    if (!communicate_kobj)
        return -ENOMEM;

    // Create the sysfs file
    error = sysfs_create_file(communicate_kobj, &stop_timer_attribute.attr);
    if (error) {
        pr_debug("failed to create the stop_timer sysfs entry\n");
        kobject_put(communicate_kobj);
    }

    return error;
}

// Exit function
static void __exit communicate_exit(void) {
    // Remove the sysfs file and the kobject
    sysfs_remove_file(communicate_kobj, &stop_timer_attribute.attr);
    kobject_put(communicate_kobj);
}

module_init(communicate_init);
module_exit(communicate_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Dejana S.");
MODULE_DESCRIPTION("Communicate Kernel Module");

