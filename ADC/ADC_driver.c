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
#define I2C_CLIENT_ADDR 0x91	// A1 & A0 pins set to GND

static struct i2c_adapter  *i2c_client_adapter = NULL;
static struct i2c_client      *i2c_client_device  = NULL;

/* turns on the A/D converter and begins conversions 
 *   SD - 1 (Single Ended Input)
 *   C2-C0 - 000 (CH0 Selected)
 *   PD1-PD0 - 11 (Internal Reference ON and A/D Converter ON)
 */
const char INIT_ADC_CONVERSION_MESSAGE               = 0x8c;

/* turns off the A/D 
 *   C2-C0 - 000
 *   PD1-PD0 - 00 (Power Down Between A/D Converter Conversions)
 */
const char SHUTDOWN_ADC_CONVERSION_MESSAGE = 0x80;

int adc_driver_major; 

/*  Each 12-bit data word is returned in two bytes */
char digital_voltage_value[2];

/* Function called when the slave has been found. */
static int driver_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	return 0;
}
/* Function called when the slave (ADC driver) has been removed */
static int driver_remove(struct i2c_client *client )
{
	i2c_master_send(i2c_client_device, &SHUTDOWN_ADC_CONVERSION_MESSAGE, 1);
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
    .probe         = driver_probe,               // @probe:     Callback for device binding. Function called when the slave has been found
    .remove      = driver_remove, 	          // @remove:  Callback for device unbinding. Function called when the slave has been removed
    .id_table      = supported_devices,    // @id_table:  List of I2C devices supported by this driver
};

static struct i2c_board_info   board_info = {
	.type = I2C_CLIENT_NAME,
	.addr = I2C_CLIENT_ADDR
};


////////////////////////////////////////// FILE OPERATIONS //////////////////////////////////////////////////////////// 
/// U Linux-u  je sve fajl. Linux periferiju/hardver vidi kao fajl.

/* This fuction will be called when we open the Device file */
static int etx_open(struct inode *inode, struct file *filp)
{
    return 0;
}
/* This fuction will be called when we close the Device file */
static int etx_release(struct inode *inode, struct file *filp)
{
    return 0;
}
/* This fuction will be called when we write the Device file */
static ssize_t etx_write(struct file *filp, const char *buf, size_t len, loff_t *off)
{
    return 0;
}

/* This fuction will be called when we read the Device file */
static ssize_t etx_read(struct file *filp, char *buf, size_t len, loff_t *f_pos)
{
    /* Size of valid data in gpio_driver - data to send in user space. */
    int data_size = 0;

    if (*f_pos == 0)
    {
		
		/* Get size of valid data. */
		data_size = strlen(digital_voltage_value);
		
		/* Slanje poruke za pokretanje ADC konverzije */
        i2c_master_send(i2c_client_device, &INIT_ADC_CONVERSION_MESSAGE, 1);
		
		/* Citanje podataka sa senzora (I2C klijenta) u 2B buffer*/
        i2c_master_recv(i2c_client_device, digital_voltage_value, data_size);
		
		/* Kopiramo podatke iz kernela u korisnički prostor */		
		if (copy_to_user(buf, digital_voltage_value, data_size) != 0)
		{
            return -EFAULT;
        }
        else
        {
            (*f_pos) += data_size;
            printk(KERN_INFO "Data_size %d\n", data_size);
            *f_pos = 0;
            return data_size;
        }
    }
    else
    {
		printk(KERN_INFO "*f_pos = %lld\n", *f_pos)
        return 0;
    }
}

/* Structure that declares the usual file access functions. */
static struct file_operations adc_fops =
{
    .owner      = THIS_MODULE,
    .read         = etx_read,			// called when we read   the Device file
    .write         = etx_write, 			// called when we write   the Device file 
    .open        = etx_open,			// called when we open  the Device file
    .release    = etx_release,       // called when we close the Device file
};


/////////////////////////////// MODULE //////////////////////////////////////////////////////////////////////////////////////////////////////

/* Module Init function */
static int      __init etx_driver_init(void)
{
    int ret = -1;
    int result = -1;
    i2c_client_adapter = i2c_get_adapter(1);
    
    if( i2c_client_adapter != NULL )
    {
        i2c_client_device = i2c_new_device(i2c_client_adapter, &board_info);
        
        if( i2c_client_device != NULL )
        {
			i2c_register_driver(THIS_MODULE, &driver);
            ret = 0;
        }
        
        i2c_put_adapter(i2c_client_adapter);
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
	printk(KERN_INFO "Removing adc_driver module\n");
		
	unregister_chrdev(adc_driver_major, "adc_driver"); // uklanjanje char uredjaja
	i2c_unregister_device(i2c_client_device);  // device destroy
	i2c_del_driver(&driver);
}

module_init(etx_driver_init);
module_exit(etx_driver_exit);
