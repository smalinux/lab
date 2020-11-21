/*
 * @file       proc_fs2.c
 * @details    Simple Linux device driver (procfs)
 * @author     smalinux
 */
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/slab.h>                 //kmalloc()
#include <linux/uaccess.h>              //copy_to/from_user()
#include <linux/ioctl.h>
#include <linux/proc_fs.h>
 
#define WR_VALUE _IOW('a','a',int32_t*)
#define RD_VALUE _IOR('a','b',int32_t*)
 
int32_t value = 0;
char sma_array[20]="try_proc_array\n";
static int len = 1;
 
 
dev_t dev = 0;
static struct class *dev_class;
static struct cdev sma_cdev;

/*
** Function Prototypes
*/
static int      __init sma_driver_init(void);
static void     __exit sma_driver_exit(void);

/*************** Driver Functions **********************/
static int      sma_open(struct inode *inode, struct file *file);
static int      sma_release(struct inode *inode, struct file *file);
static ssize_t  sma_read(struct file *filp, char __user *buf, size_t len,loff_t * off);
static ssize_t  sma_write(struct file *filp, const char *buf, size_t len, loff_t * off);
static long     sma_ioctl(struct file *file, unsigned int cmd, unsigned long arg);
 
/***************** Procfs Functions *******************/
static int      open_proc(struct inode *inode, struct file *file);
static int      release_proc(struct inode *inode, struct file *file);
static ssize_t  read_proc(struct file *filp, char __user *buffer, size_t length,loff_t * offset);
static ssize_t  write_proc(struct file *filp, const char *buff, size_t len, loff_t * off);

/*
** File operation sturcture
*/
static struct file_operations fops =
{
        .owner          = THIS_MODULE,
        .read           = sma_read,
        .write          = sma_write,
        .open           = sma_open,
        .unlocked_ioctl = sma_ioctl,
        .release        = sma_release,
};

/*
** procfs operation sturcture
*/
static struct proc_ops proc_fops = {
        .proc_open = open_proc,
        .proc_read = read_proc,
        .proc_write = write_proc,
        .proc_release = release_proc
};

/*
** This fuction will be called when we open the procfs file
*/
static int open_proc(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "proc file opend.....\t");
    return 0;
}

/*
** This fuction will be called when we close the procfs file
*/
static int release_proc(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "proc file released.....\n");
    return 0;
}

/*
** This fuction will be called when we read the procfs file
*/
static ssize_t read_proc(struct file *filp, char __user *buffer, size_t length,loff_t * offset)
{
    printk(KERN_INFO "proc file read.....\n");
    if(len)
        len=0;
    else{
        len=1;
        return 0;
    }
    copy_to_user(buffer,sma_array,20);
 
    return length;;
}

/*
** This fuction will be called when we write the procfs file
*/
static ssize_t write_proc(struct file *filp, const char *buff, size_t len, loff_t * off)
{
    printk(KERN_INFO "proc file wrote.....\n");
    copy_from_user(sma_array,buff,len);
    return len;
}

/*
** This fuction will be called when we open the Device file
*/
static int sma_open(struct inode *inode, struct file *file)
{
        printk(KERN_INFO "Device File Opened...!!!\n");
        return 0;
}

/*
** This fuction will be called when we close the Device file
*/
static int sma_release(struct inode *inode, struct file *file)
{
        printk(KERN_INFO "Device File Closed...!!!\n");
        return 0;
}

/*
** This fuction will be called when we read the Device file
*/
static ssize_t sma_read(struct file *filp, char __user *buf, size_t len, loff_t *off)
{
        printk(KERN_INFO "Readfunction\n");
        return 0;
}

/*
** This fuction will be called when we write the Device file
*/
static ssize_t sma_write(struct file *filp, const char __user *buf, size_t len, loff_t *off)
{
        printk(KERN_INFO "Write Function\n");
        return 0;
}

/*
** This fuction will be called when we write IOCTL on the Device file
*/
static long sma_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
         switch(cmd) {
                case WR_VALUE:
                        copy_from_user(&value ,(int32_t*) arg, sizeof(value));
                        printk(KERN_INFO "Value = %d\n", value);
                        break;
                case RD_VALUE:
                        copy_to_user((int32_t*) arg, &value, sizeof(value));
                        break;
        }
        return 0;
}
 
/*
** Module Init function
*/
static int __init sma_driver_init(void)
{
        /*Allocating Major number*/
        if((alloc_chrdev_region(&dev, 0, 1, "sma_Dev")) <0){
                printk(KERN_INFO "Cannot allocate major number\n");
                return -1;
        }
        printk(KERN_INFO "Major = %d Minor = %d \n",MAJOR(dev), MINOR(dev));
 
        /*Creating cdev structure*/
        cdev_init(&sma_cdev,&fops);
 
        /*Adding character device to the system*/
        if((cdev_add(&sma_cdev,dev,1)) < 0){
            printk(KERN_INFO "Cannot add the device to the system\n");
            goto r_class;
        }
 
        /*Creating struct class*/
        if((dev_class = class_create(THIS_MODULE,"sma_class")) == NULL){
            printk(KERN_INFO "Cannot create the struct class\n");
            goto r_class;
        }
 
        /*Creating device*/
        if((device_create(dev_class,NULL,dev,NULL,"sma_device")) == NULL){
            printk(KERN_INFO "Cannot create the Device 1\n");
            goto r_device;
        }
 
        /*Creating Proc entry*/
        proc_create("sma_proc",0666,NULL,&proc_fops);
 
        printk(KERN_INFO "Device Driver Insert...Done!!!\n");
        return 0;
 
r_device:
        class_destroy(dev_class);
r_class:
        unregister_chrdev_region(dev,1);
        return -1;
}
 
/*
** Module exit function
*/
static void __exit sma_driver_exit(void)
{
        remove_proc_entry("sma_proc",NULL);
        device_destroy(dev_class,dev);
        class_destroy(dev_class);
        cdev_del(&sma_cdev);
        unregister_chrdev_region(dev, 1);
        printk(KERN_INFO "Device Driver Remove...Done!!!\n");
}
 
module_init(sma_driver_init);
module_exit(sma_driver_exit);
 
MODULE_LICENSE("GPL");
MODULE_AUTHOR("smalinux <xunilams@gmail.com>");
MODULE_DESCRIPTION("Simple Linux device driver (procfs)");
MODULE_VERSION("1.6");
