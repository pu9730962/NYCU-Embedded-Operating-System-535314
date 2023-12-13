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

//LED is connected to this GPIO 1~7:7-seg, 8~10:3LED
#define GPIO_1 (1)
#define GPIO_2 (2)
#define GPIO_3 (3)
#define GPIO_4 (4)
#define GPIO_5 (5)
#define GPIO_6 (6)
#define GPIO_7 (7)
#define GPIO_8 (8) 
#define GPIO_9 (9)  
#define GPIO_10 (10)
int GPIO_arr[10]={GPIO_1,GPIO_2,GPIO_3,GPIO_4,GPIO_5,GPIO_6,GPIO_7,GPIO_8,GPIO_9,GPIO_10};

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
 uint8_t gpio_state[10]={0,0,0,0,0,0,0,0,0,0};

 //reading GPIO value
 gpio_state[0] = gpio_get_value(GPIO_1);
 gpio_state[1] = gpio_get_value(GPIO_2);
 gpio_state[2] = gpio_get_value(GPIO_3);
 gpio_state[3] = gpio_get_value(GPIO_4);
 gpio_state[4] = gpio_get_value(GPIO_5);
 gpio_state[5] = gpio_get_value(GPIO_6);
 gpio_state[6] = gpio_get_value(GPIO_7);
 gpio_state[7] = gpio_get_value(GPIO_8);
 gpio_state[8] = gpio_get_value(GPIO_9);
 gpio_state[9] = gpio_get_value(GPIO_10);
 //write to user
 len = sizeof(gpio_state);
 if( copy_to_user(buf, gpio_state, len) > 0) {
    pr_err("ERROR: Not all the bytes have been copied to user\n");
 }

 pr_info("Read function : GPIO_1,GPIO_2,GPIO_3,GPIO_4,GPIO_5,GPIO_6,GPIO_7 = %d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n",
         gpio_state[0],gpio_state[1],gpio_state[2],gpio_state[3],gpio_state[4],gpio_state[5],gpio_state[6],gpio_state[7],gpio_state[8],gpio_state[9]);
 return 0;
}
/*
** This function will be called when we write the Device file
*/
static ssize_t etx_write(struct file *filp,
                         const char __user *buf, size_t len, loff_t *off)
{
 uint8_t i;
 uint8_t c;
 uint8_t count = 0;
 int num = 0;
 int remainder[3] = {0};
 int signal[5]={0,0,0,0,0};
 int peoplenum_binary[3][7]={{0}};
 int LED8_10[3]={0};
 int zero[7]={1,1,1,1,1,1,0};
 int one[7]={0,1,1,0,0,0,0};
 int two[7]={1,1,0,1,1,0,1};
 int three[7]={1,1,1,1,0,0,1};
 int four[7]={0,1,1,0,0,1,1};
 int five[7]={1,0,1,1,0,1,1};
 int six[7]={1,0,1,1,1,1,1};
 int seven[7]={1,1,1,0,0,1,0};
 int eight[7]={1,1,1,1,1,1,1};
 int nine[7]={1,1,1,1,0,1,1};
 if( copy_from_user( signal, buf, len ) > 0) {
    pr_err("ERROR: Not all the bytes have been copied from user\n");
 }
//  for(i=0;i<5;i++){
//     printk("Signal is %d",signal[i]);
//  }
 num = signal[3];
 while(num != 0){
    remainder[count] = num%10;
    num = num/10;
    count++;
 }
 
 for(i=0;i<count;i++){
    if (remainder[count-i-1] == 0){
        memcpy(peoplenum_binary[i],zero,sizeof(zero));
    }
    else if (remainder[count-i-1] == 1){
        memcpy(peoplenum_binary[i],one,sizeof(one));
    }
    else if (remainder[count-i-1] == 2){
        memcpy(peoplenum_binary[i],two,sizeof(two));
    }
    else if (remainder[count-i-1] == 3){
        memcpy(peoplenum_binary[i],three,sizeof(three));
    }
    else if (remainder[count-i-1] == 4){
        memcpy(peoplenum_binary[i],four,sizeof(four));
    }
    else if (remainder[count-i-1] == 5){
        memcpy(peoplenum_binary[i],five,sizeof(five));
    }
    else if (remainder[count-i-1] == 6){
        memcpy(peoplenum_binary[i],six,sizeof(six));
    }
    else if (remainder[count-i-1] == 7){
        memcpy(peoplenum_binary[i],seven,sizeof(seven));
    }
    else if (remainder[count-i-1] == 8){
        memcpy(peoplenum_binary[i],eight,sizeof(eight));
    }
    else if (remainder[count-i-1] == 9){
        memcpy(peoplenum_binary[i],nine,sizeof(nine));
    }
 }
 for(i=0;i<7;i++){
    printk("%d",peoplenum_binary[0][i]);
 }
 LED8_10[0] = signal[0];
 LED8_10[1] = signal[1];
 LED8_10[2] = signal[2];

 if(signal[4] == 0){
    if(LED8_10[0] == 1){
        gpio_set_value(GPIO_8,1);
    }
    else if(LED8_10[0] == 0){
        gpio_set_value(GPIO_8,0);
    }   
    if(LED8_10[1] == 1){
        gpio_set_value(GPIO_9,1);
    }
    else if(LED8_10[1] == 0){
        gpio_set_value(GPIO_9,0);
    }
    if(LED8_10[2] == 1){
        gpio_set_value(GPIO_10,1);
    }
    else if(LED8_10[2] == 0){
        gpio_set_value(GPIO_10,0);
    }
    for(i=0;i<count;i++){
        if(peoplenum_binary[i][0] == 1){
            gpio_set_value(GPIO_1, 1);
        }
        if(peoplenum_binary[i][1] == 1){
            gpio_set_value(GPIO_2, 1);
        }
        if(peoplenum_binary[i][2] == 1){
            gpio_set_value(GPIO_3, 1);
        }
        if(peoplenum_binary[i][3] == 1){
            gpio_set_value(GPIO_4, 1);
        }
        if(peoplenum_binary[i][4] == 1){
            gpio_set_value(GPIO_5, 1);
        }
        if(peoplenum_binary[i][5] == 1){
            gpio_set_value(GPIO_6, 1);
        }
        if(peoplenum_binary[i][6] == 1){
            gpio_set_value(GPIO_7, 1);
        }
        mdelay(2000);
        gpio_set_value(GPIO_1, 0);
        gpio_set_value(GPIO_2, 0);
        gpio_set_value(GPIO_3, 0);
        gpio_set_value(GPIO_4, 0);
        gpio_set_value(GPIO_5, 0);
        gpio_set_value(GPIO_6, 0);
        gpio_set_value(GPIO_7, 0);
    }
 }
 else if(signal[4] == 1){
    for(c = 0;c < 3;c++){
        if(c == 0){
            if(LED8_10[0] == 1){
                gpio_set_value(GPIO_8,1);
            }
            if(LED8_10[1] == 1){
                gpio_set_value(GPIO_9,1);
            }
            if(LED8_10[2] == 1){
                gpio_set_value(GPIO_10,1);
            }
            for(i=0;i<count;i++){
                if(peoplenum_binary[i][0] == 1){
                    gpio_set_value(GPIO_1, 1);
                }
                if(peoplenum_binary[i][1] == 1){
                    gpio_set_value(GPIO_2, 1);
                }
                if(peoplenum_binary[i][2] == 1){
                    gpio_set_value(GPIO_3, 1);
                }
                if(peoplenum_binary[i][3] == 1){
                    gpio_set_value(GPIO_4, 1);
                }
                if(peoplenum_binary[i][4] == 1){
                    gpio_set_value(GPIO_5, 1);
                }
                if(peoplenum_binary[i][5] == 1){
                    gpio_set_value(GPIO_6, 1);
                }
                if(peoplenum_binary[i][6] == 1){
                    gpio_set_value(GPIO_7, 1);
                }
                if(count == 1){
                    mdelay(500);
                }
                else if(count == 2){
                    mdelay(250);
                }
                else if(count == 3){
                    mdelay(166);
                }
                gpio_set_value(GPIO_1, 0);
                gpio_set_value(GPIO_2, 0);
                gpio_set_value(GPIO_3, 0);
                gpio_set_value(GPIO_4, 0);
                gpio_set_value(GPIO_5, 0);
                gpio_set_value(GPIO_6, 0);
                gpio_set_value(GPIO_7, 0);
            gpio_set_value(GPIO_8, 0);
            gpio_set_value(GPIO_9, 0);
            gpio_set_value(GPIO_10,0);
            mdelay(500);       
            }
        }
        else{
            if(LED8_10[0] == 1){
                gpio_set_value(GPIO_8,1);
            }
            if(LED8_10[1] == 1){
                gpio_set_value(GPIO_9,1);
            }
            if(LED8_10[2] == 1){
                gpio_set_value(GPIO_10,1);
            }
            mdelay(500);
            gpio_set_value(GPIO_8, 0);
            gpio_set_value(GPIO_9, 0);
            gpio_set_value(GPIO_10,0);
            mdelay(500);
        }
    }
 }                   
//  pr_info("Write Function : GPIO_21 Set = %c\n", rec_buf[0]);

 return len;
}
/*
** Module Init function
*/
static int __init etx_driver_init(void)
{
 int i;
 bool gpio_valid[10]={gpio_is_valid(GPIO_1),gpio_is_valid(GPIO_2),gpio_is_valid(GPIO_3),gpio_is_valid(GPIO_4),
                     gpio_is_valid(GPIO_5),gpio_is_valid(GPIO_6),gpio_is_valid(GPIO_7),gpio_is_valid(GPIO_8),
                     gpio_is_valid(GPIO_9),gpio_is_valid(GPIO_10)};
 
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
 if((device_create(dev_class,NULL,dev,NULL,"mydev")) == NULL){
    pr_err( "Cannot create the Device \n");
    goto r_device;
 }
 
 //Checking the GPIO is valid or not
 for(i=0;i<10;i++){
    if(gpio_valid[i] == false){
        pr_err("GPIO %d is not valid\n", GPIO_arr[i]);
        goto r_device;
    }
 }

 //Requesting the GPIO
 if(gpio_request(GPIO_1,"GPIO_1") < 0){
    pr_err("ERROR: GPIO %d request\n", GPIO_1);
    goto r_gpio;
 }
 if(gpio_request(GPIO_2,"GPIO_2") < 0){
    pr_err("ERROR: GPIO %d request\n", GPIO_2);
    goto r_gpio;
 }
 if(gpio_request(GPIO_3,"GPIO_4") < 0){
    pr_err("ERROR: GPIO %d request\n", GPIO_3);
    goto r_gpio;
 } 
 if(gpio_request(GPIO_5,"GPIO_5") < 0){
    pr_err("ERROR: GPIO %d request\n", GPIO_4);
    goto r_gpio;
 }
 if(gpio_request(GPIO_6,"GPIO_6") < 0){
    pr_err("ERROR: GPIO %d request\n", GPIO_6);
    goto r_gpio;
 }
 if(gpio_request(GPIO_7,"GPIO_7") < 0){
 pr_err("ERROR: GPIO %d request\n", GPIO_7);
 goto r_gpio;
 }
 if(gpio_request(GPIO_8,"GPIO_8") < 0){
    pr_err("ERROR: GPIO %d request\n", GPIO_8);
    goto r_gpio;
 }
 if(gpio_request(GPIO_9,"GPIO_9") < 0){
    pr_err("ERROR: GPIO %d request\n", GPIO_9);
    goto r_gpio;
 }
 if(gpio_request(GPIO_10,"GPIO_10") < 0){
    pr_err("ERROR: GPIO %d request\n", GPIO_10);
    goto r_gpio;
 }

 //configure the GPIO as output
 gpio_direction_output(GPIO_1, 0);
 gpio_direction_output(GPIO_2, 0);
 gpio_direction_output(GPIO_3, 0);
 gpio_direction_output(GPIO_4, 0);
 gpio_direction_output(GPIO_5, 0); 
 gpio_direction_output(GPIO_6, 0); 
 gpio_direction_output(GPIO_7, 0); 
 gpio_direction_output(GPIO_8, 0);
 gpio_direction_output(GPIO_9, 0);
 gpio_direction_output(GPIO_10, 0);    

 /* Using this call the GPIO 21 will be visible in /sys/class/gpio/
 ** Now you can change the gpio values by using below commands also.
 ** echo 1 > /sys/class/gpio/gpio21/value (turn ON the LED)
 ** echo 0 > /sys/class/gpio/gpio21/value (turn OFF the LED)
 ** cat /sys/class/gpio/gpio21/value (read the value LED)
 **
 ** the second argument prevents the direction from being changed.
 */
 gpio_export(GPIO_1, false);
 gpio_export(GPIO_2, false); 
 gpio_export(GPIO_3, false);
 gpio_export(GPIO_4, false);
 gpio_export(GPIO_5, false);
 gpio_export(GPIO_6, false);
 gpio_export(GPIO_7, false);
 gpio_export(GPIO_8, false);
 gpio_export(GPIO_9, false);
 gpio_export(GPIO_10, false);
 pr_info("Device Driver Insert...Done!!!\n");
 return 0;
r_gpio:
    gpio_free(GPIO_1);
    gpio_free(GPIO_2);
    gpio_free(GPIO_3);
    gpio_free(GPIO_4);
    gpio_free(GPIO_5);
    gpio_free(GPIO_6);
    gpio_free(GPIO_7);
    gpio_free(GPIO_8);
    gpio_free(GPIO_9);
    gpio_free(GPIO_10);
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
 gpio_unexport(GPIO_1);
 gpio_unexport(GPIO_2);
 gpio_unexport(GPIO_3);
 gpio_unexport(GPIO_4);
 gpio_unexport(GPIO_5);
 gpio_unexport(GPIO_6);
 gpio_unexport(GPIO_7);
 gpio_unexport(GPIO_8);
 gpio_unexport(GPIO_9);
 gpio_unexport(GPIO_10); 
 gpio_free(GPIO_1);
 gpio_free(GPIO_2);
 gpio_free(GPIO_3);
 gpio_free(GPIO_4);
 gpio_free(GPIO_5);
 gpio_free(GPIO_6);
 gpio_free(GPIO_7);
 gpio_free(GPIO_8);
 gpio_free(GPIO_9);
 gpio_free(GPIO_10);
 device_destroy(dev_class,dev);
 class_destroy(dev_class);
 cdev_del(&etx_cdev);
 unregister_chrdev_region(dev, 1);
 pr_info("Device Driver Remove...Done!!\n");
}
module_init(etx_driver_init);
module_exit(etx_driver_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("CHI-CHIA,HUANG <pu9730962.ee11@nycu.edu.tw>");
MODULE_DESCRIPTION("A simple device driver - hw1");
MODULE_VERSION("1.32");
