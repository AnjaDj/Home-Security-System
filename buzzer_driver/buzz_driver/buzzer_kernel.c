 *   f_pos - a position of where to start writing in the file;
 *  Operation:
 *   The function copy_from_user transfers the data from user space to kernel space.
 */
static ssize_t buzzer_driver_write(struct file *filp, const char __user *buf, size_t len, loff_t *f_pos)
{
    int duration;

    if (copy_from_user(&duration, buf, sizeof(int)))
    {
        return -EFAULT;
    }

    //buzz(duration);
    return sizeof(int);
}

static void buzz(int duration)
{
    double period = 1 / FREQUENCY;
    period *= MULTIPLIER;
    long int end_time = jiffies + msecs_to_jiffies(duration);
    while (time_before(jiffies, end_time))
    {
        printk("U buzz funkciji smo.");
        SetGpioPin(GPIO_21);
        msleep(period/2);
        ClearGpioPin(GPIO_21);
        msleep(period/2);
    }
}