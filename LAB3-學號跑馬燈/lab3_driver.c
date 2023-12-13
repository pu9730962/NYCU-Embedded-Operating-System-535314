#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/uaccess.h> //copy_to/from_user()
#include <linux/gpio.h> //GPIO
#include <linux/slab.h>
#include <linux/delay.h>
//LED is connected to this GPIO
#define GPIO_21 (21)
#define GPIO_23 (23)
#define GPIO_24 (24)
#define GPIO_25 (25)
int GPIO_arr[4]={GPIO_21,GPIO_23,GPIO_24,GPIO_25};

dev_t dev = 0;
static struct class *dev_class;
static struct cdev etx_cdev;
static int __init etx_driver_init(void);
static void __exit etx_driver_exit(void);
/*************** Driver functions **********************/
static int etx_open(struct inode *inode, struct file *file);
static int etx_release(struct inode *inode, struct file *file);
static ssize_t etx_read(struct file *filp,char __user *buf, size_t len,loff_t * off);
static ssize_t etx_write(struct file *filp,const char *buf, size_t len, loff_t * off);
/******************************************************/
//File operation structure
static struct file_operations fops =
{
 .owner = THIS_MODULE,
 .read = etx_read,
 .write = etx_write,
 .open = etx_open,
 .release = etx_release,
};
/*
** This function will be called when we open the Device file
*/
static int etx_open(struct inode *inode, struct file *file)
{
 pr_info("Device File Opened...!!!\n");
 return 0;
}
/*
** This function will be called when we close the Device file
*/
static int etx_release(struct inode *inode, struct file *file)
{
 pr_info("Device File Closed...!!!\n");
 return 0;
}
/*
** This function will be called when we read the Device file
*/
static ssize_t etx_read(struct file *filp,
                        char __user *buf, size_t len, loff_t *off)
{
 uint8_t gpio_state[4]={0,0,0,0};

 //reading GPIO value
 gpio_state[0] = gpio_get_value(GPIO_21);
 gpio_state[1] = gpio_get_value(GPIO_23);
 gpio_state[2] = gpio_get_value(GPIO_24);
 gpio_state[3] = gpio_get_value(GPIO_25);
 //write to user
 len = sizeof(gpio_state);
 if( copy_to_user(buf, gpio_state, len) > 0) {
    pr_err("ERROR: Not all the bytes have been copied to user\n");
 }

 pr_info("Read function : GPIO_21,GPIO_23,GPIO_24,GPIO_25 = %d,%d,%d,%d\n", gpio_state[0],gpio_state[1],gpio_state[2],gpio_state[3]);
 return 0;
}
/*
** This function will be called when we write the Device file
*/
static ssize_t etx_write(struct file *filp,
                         const char __user *buf, size_t len, loff_t *off)
{
 int i;
 int a;
 uint8_t number;
 char *rec_buf;
 uint8_t inv_binary[9][4];
 rec_buf = kmalloc(len,GFP_KERNEL);
 if( copy_from_user( rec_buf, buf, len ) > 0) {
    pr_err("ERROR: Not all the bytes have been copied from user\n");
 }
 printk("the len is %ld",len);
 printk("%s",rec_buf);




 for(i=0;i<len;i++){
    number = rec_buf[i]-48;
    for(a = 0;a < 4;a++){
        inv_binary[i][a]=number%2;
        number = number/2;
        printk("%d",inv_binary[i][a]);
    }
    printk("\n");

 }
 kfree(rec_buf);
//  pr_info("Write Function : GPIO_21 Set = %c\n", rec_buf[0]);
 for(i=0;i<len;i++){
    if(inv_binary[i][0] == 1){
        // printk("25\n");
        gpio_set_value(GPIO_25, 1);
    }
    if(inv_binary[i][1] == 1){
        // printk("24\n");
        gpio_set_value(GPIO_24, 1);
    }
    if(inv_binary[i][2] == 1){
        // printk("23\n");
        gpio_set_value(GPIO_23, 1);
    }
    if(inv_binary[i][3] == 1){
        // printk("21\n");
        gpio_set_value(GPIO_21, 1);
    }
    mdelay(1000);
    gpio_set_value(GPIO_25, 0);
    gpio_set_value(GPIO_24, 0);
    gpio_set_value(GPIO_23, 0);
    gpio_set_value(GPIO_21, 0);
    }
 return len;
}
/*
** Module Init function
*/
static int __init etx_driver_init(void)
{
 int i;
 bool gpio_valid[4]={gpio_is_valid(GPIO_21),gpio_is_valid(GPIO_23),gpio_is_valid(GPIO_24),gpio_is_valid(GPIO_25)};
 
 /*Allocating Major number*/
 if((alloc_chrdev_region(&dev, 0, 1, "etx_Dev")) <0){
    pr_err("Cannot allocate major number\n");
    goto r_unreg;
 }
 pr_info("Major = %d Minor = %d \n",MAJOR(dev), MINOR(dev));
 /*Creating cdev structure*/
 cdev_init(&etx_cdev,&fops);
 /*Adding character device to the system*/
 if((cdev_add(&etx_cdev,dev,1)) < 0){
    pr_err("Cannot add the device to the system\n");
    goto r_del;
 }
 /*Creating struct class*/
 if((dev_class = class_create(THIS_MODULE,"etx_class")) == NULL){
    pr_err("Cannot create the struct class\n");
    goto r_class;
 }
 /*Creating device*/
 if((device_create(dev_class,NULL,dev,NULL,"etx_device")) == NULL){
    pr_err( "Cannot create the Device \n");
    goto r_device;
 }
 
 //Checking the GPIO is valid or not
 for(i=0;i<4;i++){
    if(gpio_valid[i] == false){
        pr_err("GPIO %d is not valid\n", GPIO_arr[i]);
        goto r_device;
    }
 }

 //Requesting the GPIO
 if(gpio_request(GPIO_21,"GPIO_21") < 0){
 pr_err("ERROR: GPIO %d request\n", GPIO_21);
 goto r_gpio;
 }
 if(gpio_request(GPIO_23,"GPIO_23") < 0){
 pr_err("ERROR: GPIO %d request\n", GPIO_23);
 goto r_gpio;
 }
 if(gpio_request(GPIO_24,"GPIO_24") < 0){
 pr_err("ERROR: GPIO %d request\n", GPIO_24);
 goto r_gpio;
 } 
 if(gpio_request(GPIO_25,"GPIO_25") < 0){
 pr_err("ERROR: GPIO %d request\n", GPIO_25);
 goto r_gpio;
 }

 //configure the GPIO as output
 gpio_direction_output(GPIO_21, 0);
 gpio_direction_output(GPIO_23, 0);
 gpio_direction_output(GPIO_24, 0);
 gpio_direction_output(GPIO_25, 0); 

 /* Using this call the GPIO 21 will be visible in /sys/class/gpio/
 ** Now you can change the gpio values by using below commands also.
 ** echo 1 > /sys/class/gpio/gpio21/value (turn ON the LED)
 ** echo 0 > /sys/class/gpio/gpio21/value (turn OFF the LED)
 ** cat /sys/class/gpio/gpio21/value (read the value LED)
 **
 ** the second argument prevents the direction from being changed.
 */
 gpio_export(GPIO_21, false);
 gpio_export(GPIO_23, false); 
 gpio_export(GPIO_24, false);
 gpio_export(GPIO_25, false);

 pr_info("Device Driver Insert...Done!!!\n");
 return 0;
r_gpio:
    gpio_free(GPIO_21);
r_device:
    device_destroy(dev_class,dev);
r_class:
    class_destroy(dev_class);
r_del:
    cdev_del(&etx_cdev);
r_unreg:
    unregister_chrdev_region(dev,1);

 return -1;
}
/*
** Module exit function
*/
static void __exit etx_driver_exit(void)
{
 gpio_unexport(GPIO_21);
 gpio_unexport(GPIO_23);
 gpio_unexport(GPIO_24);
 gpio_unexport(GPIO_25);
 gpio_free(GPIO_21);
 gpio_free(GPIO_23);
 gpio_free(GPIO_24);
 gpio_free(GPIO_25);
 device_destroy(dev_class,dev);
 class_destroy(dev_class);
 cdev_del(&etx_cdev);
 unregister_chrdev_region(dev, 1);
 pr_info("Device Driver Remove...Done!!\n");
}
module_init(etx_driver_init);
module_exit(etx_driver_exit);
MODULE_LICENSE("GPL");
// MODULE_AUTHOR("EmbeTronicX <embetronicx@gmail.com>");
MODULE_DESCRIPTION("A simple device driver - GPIO Driver");
MODULE_VERSION("1.32");
