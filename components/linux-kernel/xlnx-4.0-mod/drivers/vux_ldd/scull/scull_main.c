#include <linux/kernel.h>
#include <linux/module.h>

int scull_init_module(void) {
	printk("[vux] scull_init_module\n");

	int result, i;
	dev_t dev = 0;
	
	return 0;
}

int scull_cleanup_module(void) {
	printk("[vux] scull_cleanup_module\n");
	return 0;
}

/*Define in /linux/init*/
module_init(scull_init_module);
module_exit(scull_cleanup_module);