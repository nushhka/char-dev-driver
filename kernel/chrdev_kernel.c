#include <linux/init.h> 
#include <linux/module.h>
#include <linux/kernel.h> 
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/errno.h>
#define MAX_BUF_SIZE 256

struct cdev_data {
    struct cdev cdev;
};

static int cdev_major = 0;
static struct class *mycdev_class = NULL;
static struct cdev_data mycdev_data;
static unsigned char *user_data;

int init_module ( void ) {
    int err;
    struct device *dev_ret;
    dev_t dev;
    printk(KERN_DEBUG "Entering: %s\n", __func__);
    err = alloc_chrdev_region(&dev, 0, 1, "mycdev");
    if ( err < 0 ) {
        printk(KERN_ERR "Allocate a range of char device numbers failed.\n");
        return err;
    }
    cdev_major = MAJOR(dev);
    printk(KERN_DEBUG "device major number is: %d\n", cdev_major);
    if (IS_ERR(mycdev_class = class_create( "mycdev"))) {
        unregister_chrdev_region(MKDEV(cdev_major, 0), 1);
        return PTR_ERR(mycdev_class);
    }
   
    if (IS_ERR(dev_ret = device_create(mycdev_class, NULL, MKDEV(cdev_major, 0), NULL, "mycdev-0"))) {
        class_destroy(mycdev_class);
        unregister_chrdev_region(MKDEV(cdev_major, 0), 1);
        return PTR_ERR(dev_ret);
    }
   
    cdev_init(&mycdev_data.cdev, &cdev_fops);
    mycdev_data.cdev.owner = THIS_MODULE;
    err = cdev_add(&mycdev_data.cdev, MKDEV(cdev_major, 0), 1);
    if ( err < 0 ) {
        printk (KERN_ERR "Unable to add a char device\n");
        device_destroy(mycdev_class, MKDEV(cdev_major, 0));
        class_unregister(mycdev_class);
        class_destroy(mycdev_class);
        unregister_chrdev_region(MKDEV(cdev_major, 0), 1);        
        return err;
    }
    user_data = (unsigned char*) kzalloc(MAX_BUF_SIZE, GFP_KERNEL);
    if (user_data == NULL) {
        printk (KERN_ERR "Allocation memory for data buffer failed\n");
        device_destroy(mycdev_class, MKDEV(cdev_major, 0));
        class_unregister(mycdev_class);
        class_destroy(mycdev_class);
        unregister_chrdev_region(MKDEV(cdev_major, 0), 1);
        return -ENOMEM;
    }
    return 0;
}
void cleanup_module ( void ) {
    printk(KERN_DEBUG "Entering: %s\n", __func__);
    device_destroy(mycdev_class, MKDEV(cdev_major, 0));
    class_unregister(mycdev_class);
    class_destroy(mycdev_class);
    unregister_chrdev_region(MKDEV(cdev_major, 0), 1);
    if (user_data != NULL)
        kfree(user_data);
}
MODULE_DESCRIPTION("A simple Linux char driver");
MODULE_VERSION("1.0");
MODULE_AUTHOR("Rahul Reddy <rahulreddypurmani123@gmail.com>");
MODULE_LICENSE("GPL");