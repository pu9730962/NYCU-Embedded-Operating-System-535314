#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/uaccess.h> //copy_to/from_user()
#include <linux/slab.h>
#include <linux/delay.h>


dev_t dev = 0;
char name_char[16];   
ssize_t writed_len;
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
    int i;
    for(i = 0;i < 16;i++){
        name_char[i]=name_char[i]-48;
    }
    //write to user
    printk("%s",name_char);
    if(copy_to_user(buf,name_char,16) > 0) {
        pr_err("ERROR: Not all the bytes have been copied to user\n");
    }
    memset(name_char,'0',sizeof(name_char));
    return 0;
}
/*
** This function will be called when we write the Device file
*/
static ssize_t etx_write(struct file *filp,
                         const char __user *buf, size_t len, loff_t *off)
{
    char rec_buf ='\0';
    char none[16]="0000000000000000";
    char A_char[16]="1111001100010001";
    char B_char[16]="0000011100000101";
    char C_char[16]="1100111100000000";
    char D_char[16]="0000011001000101";
    char E_char[16]="1000011100000001";
    char F_char[16]="1000001100000001";
    char G_char[16]="1001111100010000";
    char H_char[16]="0011001100010001";
    char I_char[16]="1100110001000100";
    char J_char[16]="1100010001000100";
    char K_char[16]="0000000001101100";
    char L_char[16]="0000111100000000";
    char M_char[16]="0011001110100000";
    char N_char[16]="0011001110001000";
    char O_char[16]="1111111100000000";
    char P_char[16]="1000001101000001";
    char Q_char[16]="0111000001010000";
    char R_char[16]="1110001100011001";
    char S_char[16]="1101110100010001";
    char T_char[16]="1100000001000100";
    char U_char[16]="0011111100000000";
    char V_char[16]="0000001100100010";
    char W_char[16]="0011001100001010";
    char X_char[16]="0000000010101010";
    char Y_char[16]="0000000010100100";
    char Z_char[16]="1100110000100010";

    if( copy_from_user( &rec_buf, buf, 1 ) > 0) {
        pr_err("ERROR: Not all the bytes have been copied from user\n");
    }
    printk("the len is %ld",len);
    printk("%c",rec_buf);

    if (rec_buf == 'A'||rec_buf == 'a'){
        memcpy(name_char,A_char,sizeof(A_char));
    }
    else if (rec_buf == 'B'||rec_buf == 'b'){
        memcpy(name_char,B_char,sizeof(B_char));
    }
    else if (rec_buf == 'C'||rec_buf == 'c'){
        memcpy(name_char,C_char,sizeof(C_char));
    }
    else if (rec_buf == 'D'||rec_buf == 'd'){
        memcpy(name_char,D_char,sizeof(D_char));
    }
    else if (rec_buf == 'E'||rec_buf == 'e'){
        memcpy(name_char,E_char,sizeof(E_char));
    }
    else if (rec_buf == 'F'||rec_buf == 'f'){
        memcpy(name_char,F_char,sizeof(F_char));
    }
    else if (rec_buf == 'G'||rec_buf == 'g'){
        memcpy(name_char,G_char,sizeof(G_char));
    }
    else if (rec_buf == 'H'||rec_buf == 'h'){
        memcpy(name_char,H_char,sizeof(H_char));
    }
    else if (rec_buf == 'I'||rec_buf == 'i'){
        memcpy(name_char,I_char,sizeof(I_char));
    }
    else if (rec_buf == 'J'||rec_buf == 'j'){
        memcpy(name_char,J_char,sizeof(J_char));
    }
    else if (rec_buf == 'K'||rec_buf == 'k'){
        memcpy(name_char,K_char,sizeof(K_char));
    }
    else if (rec_buf == 'L'||rec_buf == 'l'){
        memcpy(name_char,L_char,sizeof(L_char));
    }
    else if (rec_buf == 'M'||rec_buf == 'm'){
        memcpy(name_char,M_char,sizeof(M_char));
    }
    else if (rec_buf == 'N'||rec_buf == 'n'){
        memcpy(name_char,N_char,sizeof(N_char));
    }
    else if (rec_buf == 'O'||rec_buf == 'o'){
        memcpy(name_char,O_char,sizeof(O_char));
    }
    else if (rec_buf == 'P'||rec_buf == 'p'){
        memcpy(name_char,P_char,sizeof(P_char));
    }
    else if (rec_buf == 'Q'||rec_buf == 'q'){
        memcpy(name_char,Q_char,sizeof(Q_char));
    }
    else if (rec_buf == 'R'||rec_buf == 'r'){
        memcpy(name_char,R_char,sizeof(R_char));
    }
    else if (rec_buf == 'S'||rec_buf == 's'){
        memcpy(name_char,S_char,sizeof(S_char));
    }
    else if (rec_buf == 'T'||rec_buf == 't'){
        memcpy(name_char,T_char,sizeof(T_char));
    }
    else if (rec_buf == 'U'||rec_buf == 'u'){
        memcpy(name_char,U_char,sizeof(U_char));
    }
    else if (rec_buf == 'V'||rec_buf == 'v'){
        memcpy(name_char,V_char,sizeof(V_char));
    }
    else if (rec_buf == 'W'||rec_buf == 'w'){
        memcpy(name_char,W_char,sizeof(W_char));
    }
    else if (rec_buf == 'X'||rec_buf == 'x'){
        memcpy(name_char,X_char,sizeof(X_char));
    }
    else if (rec_buf == 'Y'||rec_buf == 'y'){
        memcpy(name_char,Y_char,sizeof(Y_char));
    }
    else if (rec_buf == 'Z'||rec_buf == 'z'){
        memcpy(name_char,Z_char,sizeof(Z_char));
    }
    else{
        memcpy(name_char,none,sizeof(none));
    }

    return len;
}
/*
** Module Init function
*/
static int __init etx_driver_init(void)
{   
    memset(name_char,0,sizeof(name_char));
    /*Allocating Major number*/
    if((alloc_chrdev_region(&dev, 0, 1, "Name_LED")) <0){
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

    pr_info("Device Driver Insert...Done!!!\n");
    return 0;
    
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
MODULE_DESCRIPTION("A simple device driver - 16 levels LED Driver");
MODULE_VERSION("1.32");
