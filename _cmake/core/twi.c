/*
 * twi.c - the twi library
 *
 * Copyright (c) 2013 Parav Nagarsheth
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General
 * Public License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place, Suite 330,
 * Boston, MA  02111-1307  USA
 */

#include <twi.h>
#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include "i2c-dev.h"

#define MAX_BUF 100

struct i2c_msg twi_buf[2];
unsigned int twi_buf_count = 0;

int twi_openadapter(char* _twi_adapter)
{
	char buf[MAX_BUF];
	int twi_fd;
	snprintf(buf, sizeof(buf), "%s", _twi_adapter);
	if ((twi_fd = open(buf, O_RDWR)) < 1) {
		perror("Failed to open adapter");
		return -1;
	}
	return twi_fd;
}

int twi_setslave(int twi_fd, uint8_t addr)
{
	if (ioctl(twi_fd, I2C_SLAVE, addr) < 0) {
		fprintf(stderr, "Failed to set slave address %d:", addr);		
		perror("");
		return -1;	
	}
	return 0;
}

int twi_writebyte(int twi_fd, uint8_t byte)
{
	if (i2c_smbus_write_byte(twi_fd, byte ) < 0) {
		perror("Failed to write byte to twi slave");
		return -1;	
	}
	return 0;
}

int twi_writebytes(int twi_fd, uint8_t *bytes, uint8_t length)
{
	if(i2c_smbus_write_i2c_block_data(twi_fd, bytes[0], length-1, bytes+1) 
									< 0) {
		perror("Failed to write bytes to twi slave");
		return -1;
	}
	return 0;
}

int twi_readbyte(int twi_fd)
{
	int byte;
	if ((byte = i2c_smbus_read_byte(twi_fd)) < 0) {
		// perror("Failed to read byte from twi slave");
		return -1;
	}
	return byte;
}

int twi_readbytes(int twi_fd, uint8_t *buf, int length)
{
	int ret = i2c_smbus_read_i2c_block_data(twi_fd, 0, length, buf);
	return ret;
}

int twi_readwrite(int twi_fd)
{
	struct i2c_rdwr_ioctl_data packets;
	packets.msgs = twi_buf;
	packets.nmsgs = twi_buf_count;
	if (ioctl(twi_fd, I2C_RDWR, &packets) < 0) {
		perror("Unable to send data");
		twi_buf_count = 0;
		return -1;
	}
	twi_buf_count = 0;
	return 0;
}

int twi_add_to_buf(uint8_t addr, uint8_t rw, uint8_t *value, int length)
{
	if(twi_buf_count < 2) {
		twi_buf[twi_buf_count].addr = addr;
		twi_buf[twi_buf_count].flags = rw ? I2C_M_RD : 0;
		twi_buf[twi_buf_count].len = length;
		twi_buf[twi_buf_count].buf = (char *)value;
		return ++twi_buf_count;
	} else
		return -1;
}
