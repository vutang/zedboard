#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>		/* everything... */
#include <linux/cdev.h>

int scull_major =   0;
int scull_minor =   0;
int scull_nr_devs = 4;	/* number of bare scull devices */

struct cdev scull_cdev[4];	  /* Char device structure		*/

int scull_open(struct inode *inode, struct file *filp) {
	printk("skull_open <%d, %d>\n", MAJOR(inode->i_cdev->dev), MINOR(inode->i_cdev->dev));
	return 0;
}

int scull_release(struct inode *inode, struct file *filp) {
	printk("skull_release\n");
	return 0;
}

ssize_t scull_read(struct file *filp, char __user *buf, size_t count,
                loff_t *f_pos) {
	printk("scull_read: %d\n", count);
	return 0;
}

ssize_t scull_write(struct file *filp, const char __user *buf, size_t count,
                loff_t *f_pos){
	printk("scull_write\n");
	return count;
}
struct file_operations scull_fops = {
	.owner =    THIS_MODULE,
	// .llseek =   scull_llseek,
	.read =     scull_read,
	.write =    scull_write,
	// .ioctl =    scull_ioctl,
	.open =     scull_open,
	.release =  scull_release,
};

int scull_init_module(void) {
	printk("[vux] scull_init_module\n");

	int result, i, err;
	dev_t dev = 0;

	if (scull_major) {
		dev = MKDEV(scull_major, scull_minor);
		result = register_chrdev_region(dev, scull_nr_devs, "scull");
	} 
	else {
		printk("Dynamic Allocate major\n");
		result = alloc_chrdev_region(&dev, scull_minor, scull_nr_devs,
				"scull");
		scull_major = MAJOR(dev);
	}

	printk("[vux] register scull_major %d\n", scull_major);

	if (result < 0) {
		printk(KERN_WARNING "scull: can't get major %d\n", scull_major);
		return result;
	}

	dev_t devno;
	for (i = 0; i < scull_nr_devs; i++) {
		devno = MKDEV(scull_major, i);
		cdev_init(&scull_cdev[i], &scull_fops);
		err = cdev_add(&scull_cdev[i], devno, 1);
		/* Fail gracefully if need be */
		if (err)
			printk(KERN_NOTICE "Error %d adding scull", err);
	}
	return 0;
}

int scull_cleanup_module(void) {
	dev_t devno = MKDEV(scull_major, scull_minor);
	printk("[vux] scull_cleanup_module\n");
	/* cleanup_module is never called if registering failed */
	unregister_chrdev_region(devno, scull_nr_devs);
	return 0;
}

/*Define in /linux/init*/
module_init(scull_init_module);
module_exit(scull_cleanup_module);