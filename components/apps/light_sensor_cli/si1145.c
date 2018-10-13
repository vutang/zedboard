/*For monitor SI1145 via I2C bus*/

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
#include "si1145.h"

#define I2C_SI1145_DEV_PATH "/dev/i2c-0"
#define I2C_SI1145_ADDR 0x60
int i2c_si1145_fd;

typedef struct si1145_dat {
	int uv_data;
	// u_int32_t temp_c;
} si1145_dat_t;

int si1145_dev_open() {
	int ret, file;
	file = open(I2C_SI1145_DEV_PATH, O_RDWR);
	if (file < 0) {
		LOG_ERROR("Cannot open %s errno = %s", I2C_SI1145_DEV_PATH, strerror(errno));
		return -1;
	}
	ret = ioctl(file, I2C_SLAVE_FORCE, I2C_SI1145_ADDR);
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
	LOG_DEBUG("Open %s success (Device ID: 0x%20x)", I2C_SI1145_DEV_PATH, ret);
	i2c_si1145_fd = file;
	return 0;
}

int si1145_dev_close() {
	if (i2c_si1145_fd > 0) 
		close(i2c_si1145_fd);
	return 0;
}

int si1145_dev_write_byte(unsigned char reg, unsigned char value) {
	int ret = -1;
	if (i2c_si1145_fd < 0) {
		LOG_ERROR("Device is not open yet");
		return -1;
	}
	if(ioctl(i2c_si1145_fd, I2C_SLAVE_FORCE, I2C_SI1145_ADDR)<0) {
		LOG_ERROR("Fail to set slave address. Exit");
		return -2;
 	}
	ret = i2c_smbus_write_byte_data(i2c_si1145_fd, reg, value);	
	return ret;
}

int si1145_dev_read_byte(unsigned char reg, unsigned char *ret_value) {
	int ret = -1;
	unsigned char tmp;
	
	if (i2c_si1145_fd < 0) {
		LOG_ERROR("i2c dev file is not opened yet");
		return -1;
	}
	if(ioctl(i2c_si1145_fd, I2C_SLAVE_FORCE, I2C_SI1145_ADDR)<0) {
		LOG_ERROR("Fail to set slave address. Exit");
		return -2;
 	}
	tmp = i2c_smbus_read_byte_data(i2c_si1145_fd, reg);
	if(tmp < 0) {
		LOG_ERROR("Read smbus fail");
		return -3;
	}
	else {
		*ret_value = tmp;
		return 0;
	}
}	

int si1145_dev_read_word(unsigned char reg, unsigned short *ret_value) {
	int ret = -1;
	unsigned short tmp;
	
	if (i2c_si1145_fd < 0) {
		LOG_ERROR("i2c dev file is not opened yet");
		return -1;
	}
	if(ioctl(i2c_si1145_fd, I2C_SLAVE_FORCE, I2C_SI1145_ADDR)<0) {
		LOG_ERROR("Fail to set slave address. Exit");
		return -2;
 	}
	tmp = i2c_smbus_read_word_data(i2c_si1145_fd, reg);
	if(tmp < 0) {
		LOG_ERROR("Read smbus fail");
		return -3;
	}
	else {
		*ret_value = tmp;
		return 0;
	}
}

int si1145_get_uvdata(int *ret_value) {
	unsigned char rx_uv_data[2];
	int ret;
	ret = si1145_dev_read_byte(0x2C, &rx_uv_data[0]);
	if (ret != 0) {
		LOG_ERROR("Read LSB UV Fail");
		return -1;
	}

	ret = si1145_dev_read_byte(0x2D, &rx_uv_data[1]);
	if (ret != 0) {
		LOG_ERROR("Read MSB UV Fail");
		return -1;
	}
	LOG_DEBUG("rx_uv_data[0]: 0x%x, rx_uv_data[1]: 0x%x", rx_uv_data[0], rx_uv_data[1]);
	*ret_value = ((rx_uv_data[1] << 8) | rx_uv_data[0] + 50)/1000;
	return 0;
}