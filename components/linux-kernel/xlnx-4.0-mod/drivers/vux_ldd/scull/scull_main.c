#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>		/* everything... */

int scull_major =   0;
int scull_minor =   0;
int scull_nr_devs = 4;	/* number of bare scull devices */

int scull_init_module(void) {
	printk("[vux] scull_init_module\n");

	int result, i;
	dev_t dev = 0;

	if (scull_major) {
		dev = MKDEV(scull_major, scull_minor);
		result = register_chrdev_region(dev, scull_nr_devs, "scull");
	} 
	else {
		result = alloc_chrdev_region(&dev, scull_minor, scull_nr_devs,
				"scull");
		scull_major = MAJOR(dev);
	}

	printk("[vux] register scull_major %d\n", scull_major);

	if (result < 0) {
		printk(KERN_WARNING "scull: can't get major %d\n", scull_major);
		return result;
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