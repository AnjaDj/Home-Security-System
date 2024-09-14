#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/uaccess.h>

#define I2C_CLIENT_NAME "CLIENT_ADC"
#define I2C_CLIENT_ADDR 0x48

static struct i2c_adapter* client_adapter = NULL;
static struct i2c_client*     client_device   = NULL;
const char INIT_MSG = 0x8c; // Message that initiates conversion for ADC 12 Click component
const char device_removed = 0x80;
int adc_driver_major; 
char data[2]; // Buffer holding read data from the ADC (sensor data)

static int driver_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	return 0;
}
static int driver_remove(struct i2c_client *client )
{
	i2c_master_send(client_device, &device_removed, 1);
	return 0;
}

static const struct i2c_device_id   supported_devices[] = {
    { I2C_CLIENT_NAME, 0 },
    { }
};
MODULE_DEVICE_TABLE(i2c, supported_devices);

static struct i2c_driver   driver = {
    .driver = {
        .name   = I2C_CLIENT_NAME,
        .owner  = THIS_MODULE,
    },
    .probe         = driver_probe,               // @probe:    Callback for device binding. Function called when the slave has been found.
    .remove      = driver_remove, 	       // @remove: Callback for device unbinding. Function called when the slave has been removed.@id_table: List of I2C devices supported by this driver
    .id_table      = supported_devices,    // @id_table:  List of I2C devices supported by this driver
};

static struct i2c_board_info   board_info = {
	.type = I2C_CLIENT_NAME,
	.addr = I2C_CLIENT_ADDR
};


///////////////////////// FILE OPERATIONS ///////////////////////////////

/* This fuction will be called when we open the Device file */
static int etx_open(struct inode *inode, struct file *file)
{
    return 0;
}

/* This fuction will be called when we close the Device file */
static int etx_release(struct inode *inode, struct file *file)
{
    return 0;
}

/* This fuction will be called when we read the Device file */
static ssize_t etx_read(struct file *filp, char *buf, size_t len, loff_t *f_pos)
{
    int data_size = 0;

    if (*f_pos == 0)
    {
		/* Writting INIT_MSG before every read operation */
		/* Writes the data into the I2C client */
        i2c_master_send(client_device, &INIT_MSG, 1);
		
		/* Reading sensor data into the buffer, 2B of data */
		/* Reads 2 bytes of the data from the I2C client */
        i2c_master_recv(client_device, data, 2);
		
        if (copy_to_user(buf, data, 2) != 0)
        {
            return -EFAULT;
        }
        else
        {
            (*f_pos) += data_size;

            return data_size;
        }
    }
    else
    {
        return 0;
    }
}

/* This fuction will be called when we write the Device file */
static ssize_t etx_write(struct file *filp, const char *buf, size_t len, loff_t *off)
{
    return len;
}

static struct file_operations adc_fops =
{
    .owner      = THIS_MODULE,
    .read         = etx_read,
    .write         = etx_write,
    .open        = etx_open,
    .release    = etx_release,
};

/* Module Init function */
static int      __init etx_driver_init(void)
{
    int ret = -1;
    int result = -1;
    client_adapter = i2c_get_adapter(1);
    
    if( client_adapter != NULL )
    {
        client_device = i2c_new_device(client_adapter, &board_info);
        
        if( client_device != NULL )
        {
			i2c_register_driver(THIS_MODULE, &driver);
            ret = 0;
        }
        
        i2c_put_adapter(client_adapter);
    }

    /* Registering device. */
    result = register_chrdev(0, "adc_driver", &adc_fops);
    if (result < 0)
    {
        printk(KERN_INFO "adc_driver: cannot obtain major number %d\n", adc_driver_major);
        return result;
    }

    adc_driver_major = result;
    printk(KERN_INFO "adc_driver major number is %d\n", adc_driver_major);

    return ret;
}

/* Module exit function */
static void __exit etx_driver_exit(void)
{
	i2c_unregister_device(client_device);  // device destroy
	i2c_del_driver(&driver);
	i2c_del_adapter(client_adapter);
	unregister_chrdev(adc_driver_major, "adc_driver");
    // printk(KERN_INFO "Device Driver Remove...Done!!!\n");
}

module_init(etx_driver_init);
module_exit(etx_driver_exit);
