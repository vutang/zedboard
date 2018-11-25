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
#include <linux/platform_device.h>

#include <linux/uaccess.h> /*Copy from/to user*/
#include <asm/io.h>	/*iowrite32/read*/

MODULE_LICENSE("GPL");

#define DRIVER_NAME "plcore"
#define PLCORE_ID_TIMER 0
#define PLCORE_ID_LED 1

int plcore_major =   0;
int plcore_minor =   0;
int plcore_nr_devs = 4;	/* number of bare plcore devices */

struct cdev plcore_cdev[4];
struct class *plcore_cl;

/*Struct to hold private info for driver*/
struct plcore_priv {
	/*Hold device file info*/
	struct cdev cdev;
	dev_t devt;
	
	/*Memory*/
	void __iomem *base;

	/*Interrupt*/
	int irq;
	// int (*irq_handler)();
};

/*For IOCTL*/
enum plcore_ioc_cmd_id {
	PLCORE_IOC_REG_READ = 100,
	PLCORE_IOC_REG_WRITE,
};

struct  plcore_ioc_cmd {
	unsigned addr;
	unsigned value;
};

static struct plcore_priv plcore_priv_list[4];

static int plcore_probe(struct platform_device *pdev);
static int plcore_remove(struct platform_device *pdev);

/*Driver Match
	struct of_device_id is defined in linux/mod_devicetable.h
*/
static struct of_device_id plcore_of_match[] = {
	{ .compatible = "xlnx,vux-timer-1.0", .data = PLCORE_ID_TIMER},
	{ .compatible = "xlnx,xps-vgpio-1.00.a", .data = PLCORE_ID_LED},
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

/*File Operations*/
int plcore_open(struct inode *inode, struct file *filp) {
	/*Get dev_t from inode from i_cdev or i_rdev, in linux/fs.h defines a 
	function to get major and minor from inode pointer*/
	printk("plcore_open <%d, %d>\n", imajor(inode), iminor(inode));
	return 0;
}

int plcore_release(struct inode *inode, struct file *filp) {
	printk("plcore_release\n");
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

int plcore_ioctl(struct file *filep, unsigned int cmd, unsigned long arg) {
	int id = iminor(filep->f_path.dentry->d_inode);
	pr_info("Receive IOCTL from <%d, %d>\n", imajor(filep->f_path.dentry->d_inode), \
			iminor(filep->f_path.dentry->d_inode));

	struct plcore_ioc_cmd ioc_cmd;
	copy_from_user(&ioc_cmd, (struct plcore_ioc_cmd *) arg, 
		sizeof(struct plcore_ioc_cmd));

	switch(cmd){
		case PLCORE_IOC_REG_READ: {
			break;
		}
		case PLCORE_IOC_REG_WRITE: {
			iowrite32(ioc_cmd.value, \
				(u32 *) (plcore_priv_list[id].base + ioc_cmd.addr));
			break;
		}
		default: 
			printk("unknown command");
			break;
	}

	return 0;
}

struct file_operations plcore_fops = {
	.owner =    		THIS_MODULE,
	.read =     		plcore_read,
	.write =    		plcore_write,
	.unlocked_ioctl =   plcore_ioctl,
	.open =     		plcore_open,
	.release =  		plcore_release,
};

/*Driver operations
	struct platform_device is defined in platform_device.h
*/
static int plcore_probe(struct platform_device *pdev) {
	struct of_device_id *of_dev_id;
	dev_t devt;
	int err, id;

	struct resource *mem;
	void __iomem	*base;

	/*of_match_device is defined in of/device.c, use to find 
	of_device in of_device_id table	that is probing*/
	of_dev_id = of_match_device(plcore_of_match, &pdev->dev);
	if (of_dev_id == NULL) {
		dev_err(&pdev->dev, "of_device_id fail");
		return -1;
	}
	id = (int) of_dev_id->data;
	dev_info(&pdev->dev, "probing plcore id = %d (%s)\n", id, \
		of_dev_id->compatible);

	/*Get memory resource*/
	mem = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (mem == NULL) {
		dev_err(&pdev->dev, "platform_get_resource fail");
	}

	dev_info(&pdev->dev, "get resource %s success\n", mem->name);

	base = devm_ioremap_resource(&pdev->dev, mem);
	if (IS_ERR(base))
		return PTR_ERR(base);
	plcore_priv_list[id].base = base;

	/*Time to create Device File*/
	devt = MKDEV(plcore_major, id);
	dev_info(&pdev->dev, "create device <%d, %d>\n", MAJOR(devt), MINOR(devt));

	plcore_priv_list[id].devt = devt;
	cdev_init(&plcore_cdev[(int) of_dev_id->data], &plcore_fops);
	err = cdev_add(&plcore_cdev[(int) of_dev_id->data], devt, 1);
	if (err)
		printk(KERN_NOTICE "Error %d adding plcore", err);

	if (device_create(plcore_cl, NULL, devt, NULL, \
			"plcore-%d", id) == NULL) {
		return -1;
	}

	return 0;
}

static int plcore_remove(struct platform_device *pdev) {
	int id;
	struct of_device_id *of_dev_id = of_match_device(plcore_of_match, &pdev->dev);
	if (of_dev_id == NULL) {
		dev_err(&pdev->dev, "remove fail at of_device_id");
		return -1;
	}
	id = (int) of_dev_id->data;
	dev_info(&pdev->dev, "remove plcore id = %d (%s)\n", id, \
		of_dev_id->compatible);

	device_destroy(plcore_cl, plcore_priv_list[id].devt);
	return 0;
}

int plcore_init_module(void) {
	printk("[vux] plcore_init_module\n");

	int result, i, err;
	dev_t dev = 0;

	if (plcore_major) {
		dev = MKDEV(plcore_major, plcore_minor);
		result = register_chrdev_region(dev, plcore_nr_devs, "plcore");
	} 
	/*Automatically allocate major number*/
	else {
		result = alloc_chrdev_region(&dev, plcore_minor, plcore_nr_devs,
				"plcore");
		plcore_major = MAJOR(dev);
	}

	printk("[vux] register plcore_major %d\n", plcore_major);

	if (result < 0) {
		printk(KERN_WARNING "plcore: can't get major %d\n", plcore_major);
		return result;
	}

	if ((plcore_cl = class_create(THIS_MODULE, "plcore")) == NULL) {
		unregister_chrdev_region(dev, 1);
		printk("Fail to class_create()\n");
		return -1;
	}

	return platform_driver_register(&plcore_driver);
}

int plcore_cleanup_module(void) {
	dev_t dev = MKDEV(plcore_major, plcore_minor);
	/* cleanup_module is never called if registering failed */
	printk("[vux] plcore platform_driver_unregister\n");
	platform_driver_unregister(&plcore_driver);
	class_destroy(plcore_cl);
	printk("[vux] plcore unregister_chrdev_region\n");
	unregister_chrdev_region(dev, plcore_nr_devs);
	return 0;
}

/*Define in /linux/init*/
module_init(plcore_init_module);
module_exit(plcore_cleanup_module);