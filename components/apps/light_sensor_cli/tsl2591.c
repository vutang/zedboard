/*
* @Author: vutang
* @Date:   2018-10-20 15:11:55
* @Last Modified by:   vutang
* @Last Modified time: 2018-11-01 09:31:09
*/
#include <sys/ioctl.h>
#include <stdlib.h>
#include <error.h>
#include <errno.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <linux/i2c-dev.h>

#include "logger.h"
#include "smbus.h"
#include "tsl2591.h"

#define I2C_TSL2591_DEV_PATH "/dev/i2c-1"
#define I2C_TSL2591_ADDR 0x29 /*Fixed in Datasheet*/

int i2c_tsl2591_fd = -1;

int tsl2591_dev_open() {
	int ret, file;
	file = open(I2C_TSL2591_DEV_PATH, O_RDWR);
	if (file < 0) {
		LOG_ERROR("Cannot open %s errno = %s", I2C_TSL2591_DEV_PATH, strerror(errno));
		return -1;
	}
	ret = ioctl(file, I2C_SLAVE_FORCE, I2C_TSL2591_ADDR);
	if (ret < 0) {
		LOG_ERROR("Set I2C_SLAVE_FORCE fail errno = %s", strerror(errno));
		return -1;
	}

	/*Try to read path ID*/
	ret = i2c_smbus_read_byte_data(file, 0x00);
	if (ret < 0) {
		LOG_ERROR("Read data from i2c bus fail errno = %s", strerror(errno));
		return -1;
	}
	LOG_DEBUG("Open %s success (Device ID: 0x%20x)", I2C_TSL2591_DEV_PATH, ret);
	i2c_tsl2591_fd = file;
	return 0;
}

int tsl2591_dev_close() {
	if (i2c_tsl2591_fd > 0) 
		close(i2c_tsl2591_fd);
	return 0;
}

int tsl2591_dev_write_byte(unsigned char reg, unsigned char value) {
	int ret = -1;
	unsigned char cmd;
	if (i2c_tsl2591_fd < 0) {
		LOG_ERROR("Device is not open yet");
		return -1;
	}
	if(ioctl(i2c_tsl2591_fd, I2C_SLAVE_FORCE, I2C_TSL2591_ADDR)<0) {
		LOG_ERROR("Fail to set slave address. Exit");
		return -2;
 	}

 	/*Normal operation, this information can be read from tls2591 
 	datasheet. Format:
	|CMD[7:7]|TRANSACTION[6:5]|ADDR/SF[4:0]|
 	*/
	ret = i2c_smbus_write_byte_data(i2c_tsl2591_fd, reg, value);	
	return ret;
}

int tsl2591_dev_read_byte(unsigned char reg, unsigned char *ret_value) {
	int ret = -1;
	unsigned char cmd, tmp;
	
	if (i2c_tsl2591_fd < 0) {
		LOG_ERROR("i2c dev file is not opened yet");
		return -1;
	}
	if(ioctl(i2c_tsl2591_fd, I2C_SLAVE_FORCE, I2C_TSL2591_ADDR)<0) {
		LOG_ERROR("Fail to set slave address. Exit");
		return -2;
 	}

	tmp = i2c_smbus_read_byte_data(i2c_tsl2591_fd, reg);
	if(tmp < 0) {
		LOG_ERROR("Read smbus fail");
		return -3;
	}
	else {
		*ret_value = tmp;
		return 0;
	}
}	

/*Enables the chip, so it's ready to take readings*/
int tsl2591_enable() {
	LOG_DEBUG("Enable TSL2591");
	return tsl2591_dev_write_byte(TSL2591_COMMAND_BIT | TSL2591_REGISTER_ENABLE,\
			TSL2591_ENABLE_POWERON | TSL2591_ENABLE_AEN | TSL2591_ENABLE_AIEN | \
			TSL2591_ENABLE_NPIEN);
}