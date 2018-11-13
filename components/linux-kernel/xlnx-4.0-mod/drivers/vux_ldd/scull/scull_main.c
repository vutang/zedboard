/*
* @Author: Vu Tang
* @Date:   2018-11-13 22:20:13
* @Last Modified by:   Vu Tang
* @Last Modified time: 2018-11-13 22:58:15
*/
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>		/* everything... */
#include <linux/cdev.h>

#include <linux/of_device.h>
#include <linux/of_platform.h>

#define DRIVER_NAME "plcore"
#define PLCORE_ID_TIMER 0

int plcore_major =   0;
int plcore_minor =   0;
int plcore_nr_devs = 4;	/* number of bare plcore devices */

/*Struct to hold private info for driver*/
struct plcore_priv {
	/*Hold device file info*/
	struct cdev cdev_;
	
	/*Memory*/
	void __iomem *base;

	/*Interrupt*/
	int irq;
	int (*irq_handler)();
};

/*File Operations*/
int plcore_open(struct inode *inode, struct file *filp) {
	printk("skull_open <%d, %d>\n", MAJOR(inode->i_cdev->dev), MINOR(inode->i_cdev->dev));
	return 0;
}

int plcore_release(struct inode *inode, struct file *filp) {
	printk("skull_release\n");
	return 0;
}

ssize_t plcore_read(struct file *filp, char __user *buf, size_t count,
                loff_t *f_pos) {
	printk("plcore_read: %d\n", count);
	return 0;
}

ssize_t plcore_write(struct file *filp, const char __user *buf, size_t count,
                loff_t *f_pos){
	printk("plcore_write\n");
	return count;
}
struct file_operations plcore_fops = {
	.owner =    THIS_MODULE,
	// .llseek =   plcore_llseek,
	.read =     plcore_read,
	.write =    plcore_write,
	// .ioctl =    plcore_ioctl,
	.open =     plcore_open,
	.release =  plcore_release,
};

/*Driver operations
	struct platform_device is defined in platform_device.h
*/
static int plcore_probe(struct platform_device *pdev) {
	struct of_device_id *of_dev_id;
	/*of_match_device is defined in of/device.c, use to find of_device in of_device_id table
	that is probing*/
	of_dev_id = of_match_device(plcore_of_match, pdev->dev);
	if (of_dev_id == NULL) {
		dev_err(pdev->dev, "of_device_id fail");
		return -1;
	}
	dev_info(pdev->dev, "probing plcore id = %d (%s)\n", (int) of_dev_id->data, \
		of_dev_id->compatible);

	/*Time to create Device File*/
	// Create Device Here
	return 0;
}

static int plcore_remove(struct platform_device *pdev) {
	printk("plcore remove\n");
	return 0;
}

/*Driver Match
	struct of_device_id is defined in linux/mod_devicetable.h
*/
static struct of_device_id plcore_of_match[] = {
	{ .compatible = "xlnx,ZedboardOLED-1.0", .data = PLCORE_ID_TIMER},
	{ /* end of list */ },
};

static struct platform_driver plcore_driver = {
	.driver = {
		.name = DRIVER_NAME,
		.owner = THIS_MODULE,
		.of_match_table	= plcore_of_match,
	},
	.probe		= plcore_probe,
	.remove		= plcore_remove,
};

int plcore_init_module(void) {
	printk("[vux] plcore_init_module\n");

	int result, i, err;
	dev_t dev = 0;

	if (plcore_major) {
		dev = MKDEV(plcore_major, plcore_minor);
		result = register_chrdev_region(dev, plcore_nr_devs, "plcore");
	} 
	else {
		printk("Dynamic Allocate major\n");
		result = alloc_chrdev_region(&dev, plcore_minor, plcore_nr_devs,
				"plcore");
		plcore_major = MAJOR(dev);
	}

	printk("[vux] register plcore_major %d\n", plcore_major);

	if (result < 0) {
		printk(KERN_WARNING "plcore: can't get major %d\n", plcore_major);
		return result;
	}

	dev_t devno;
	for (i = 0; i < plcore_nr_devs; i++) {
		devno = MKDEV(plcore_major, 0);
		cdev_init(&plcore_cdev[0], &plcore_fops);
		err = cdev_add(&plcore_cdev[0], devno, 4);
		/* Fail gracefully if need be */
		if (err)
			printk(KERN_NOTICE "Error %d adding plcore", err);
	}

	return platform_driver_register(&plcore_driver);
}

int plcore_cleanup_module(void) {
	dev_t devno = MKDEV(plcore_major, plcore_minor);
	printk("[vux] plcore_cleanup_module\n");
	/* cleanup_module is never called if registering failed */
	unregister_chrdev_region(devno, plcore_nr_devs);
	return 0;
}

/*Define in /linux/init*/
module_init(plcore_init_module);
module_exit(plcore_cleanup_module);