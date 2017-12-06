/*
  sysfs.c

  Copyright (c) 2013 Anuj Deshpande

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General
  Public License along with this library; if not, write to the
  Free Software Foundation, Inc., 59 Temple Place, Suite 330,
  Boston, MA  02111-1307  USA

*/

#include "sysfs.h"
#include "virtual_main.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>
#include "pins_arduino.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#ifndef MRAA_BACKEND

int sysfs_read(const char *path, const char *filename, char *value)
{
	FILE *fd;
	char buf[MAX_BUF];
	snprintf(buf, sizeof(buf), "%s%s", path, filename);
	fd = fopen(buf, "r");
	if (fd == NULL) {
		perror(buf);
		return -1;
	}
	fscanf(fd, "%s", value);
	if (fclose(fd) != 0)
		perror(buf);
	return 0;
}

int sysfs_write(const char *path, const char *filename, int value)
{
	FILE *fd;
	char buf[MAX_BUF];
	snprintf(buf, sizeof(buf), "%s%s", path, filename);
	fd = fopen(buf, "w");
	if (fd == NULL) {
		perror(buf);
		return -1;
	}
	fprintf(fd, "%d", value);
	if (fclose(fd) != 0)
		perror(buf);
	return 0;
}

/* sysfs_adc_getvalue
 * Gets a value from the sysfs adc entries
 * Returns a value on the scale of (0 to 4095)
 */
uint32_t sysfs_adc_getvalue(uint32_t channel)
{
	char buf[MAX_BUF], channelname[20], value[8];
	snprintf(buf, sizeof(buf), SYSFS_ADC_PATH);
	snprintf(channelname, sizeof(channelname), "in_voltage%d_raw", channel);
	sysfs_read(buf, channelname, value);
	return atol(value);
}

int gpio_export(uint32_t gpio_pin)
{
	FILE *fd;
	fd = fopen(SYSFS_GPIO_PATH"/export", "w");
	if (fd == NULL) {
		fprintf(stderr, "Pin %d: ", gpio_pin);
		perror("/gpio/export");
		return gpio_pin;
	}
	fprintf(fd, "%d", gpio_pin);
	if (fclose(fd) != 0) {
		fprintf(stderr, "Pin %d: ", gpio_pin);
		perror("/gpio/export");
	}
	return gpio_pin;
}

int gpio_unexport(uint32_t gpio_pin)
{
	FILE *fd;
	fd = fopen(SYSFS_GPIO_PATH"/unexport", "w");
	if (fd == NULL) {
		fprintf(stderr, "Pin %d: ", gpio_pin);
		perror("/gpio/unexport");
		return -1;
	}
	fprintf(fd, "%d", gpio_pin);
	if (fclose(fd) != 0) {
		fprintf(stderr, "Pin %d: ", gpio_pin);
		perror("/gpio/unexport");
	}
	return gpio_pin;
}

int gpio_setdirection(uint32_t gpio_pin, const char *direction)
{
	FILE *fd;
	char buf[MAX_BUF];
	snprintf(buf, sizeof(buf), SYSFS_GPION_PATH"/direction", gpio_pin);
	fd = fopen(buf, "w");
	if (fd == NULL) {
		fprintf(stderr, "Pin %d: ", gpio_pin);
		perror(buf);
		return -1;
	}
	fprintf(fd, "%s", direction);
	if (fclose(fd) != 0) {
		fprintf(stderr, "Pin %d: ", gpio_pin);
		perror(buf);
	}
	return gpio_pin;
}

int gpio_setedge(unsigned int gpio_pin, const char *edge)
{
	FILE *fd;
	char buf[MAX_BUF];
	snprintf(buf, sizeof(buf), SYSFS_GPION_PATH"/edge", gpio_pin);
	fd = fopen(buf, "w");
	if (fd == NULL) {
		fprintf(stderr, "Pin %d: ", gpio_pin);
		perror(buf);
		return -1;
	}
	fprintf(fd, "%s", edge);
	if (fclose(fd) != 0) {
		fprintf(stderr, "Pin %d: ", gpio_pin);
		perror(buf);
	}
	return gpio_pin;
}

int gpio_fd_open(unsigned int gpio_pin)
{
	int fd;
	char buf[MAX_BUF];

	snprintf(buf, sizeof(buf), SYSFS_GPION_PATH"/value", gpio_pin);

	fd = open(buf, O_RDONLY | O_NONBLOCK );
	if (fd < 0) {
		perror("gpio/fd_open");
	}
	return fd;
}

int gpio_fd_close(int fd)
{
	return close(fd);
}

int pwm_export(uint32_t pwm_pin)
{
	FILE *fd;
	char buf[MAX_BUF];
	fd = fopen(SYSFS_PWM_PATH"/export", "w");
	if (fd == NULL) {
		fprintf(stderr, "Pin %d: ", pwm_pin);
		perror("error exporting pwm");
		return -1;
	}
	if (g_APinDescription[pwm_pin].pinType | PWM) {
		snprintf(buf, sizeof(buf), "%d", (g_APinDescription[pwm_pin].pinType >> PWM_N) & 0xF);
		fprintf(fd, buf);
	}
	if (fclose(fd) != 0) {
		fprintf(stderr, "Pin %d: ", pwm_pin);
		perror("error exporting pwm");
	}
	return pwm_pin;
}

int pwm_unexport(uint32_t pwm_pin)
{
	FILE *fd;
	char buf[MAX_BUF];
	fd = fopen(SYSFS_PWM_PATH"/unexport", "w");
	if (fd == NULL) {
		fprintf(stderr, "Pin %d: ", pwm_pin);
		perror("error exporting pwm");
		return -1;
	}
	if (g_APinDescription[pwm_pin].pinType | PWM) {
		snprintf(buf, sizeof(buf), "%d", (g_APinDescription[pwm_pin].pinType >> PWM_N) & 0xF);
		fprintf(fd, buf);
	}
	if (fclose(fd) != 0) {
		fprintf(stderr, "Pin %d: ", pwm_pin);
		perror("error exporting pwm");
	}
	return pwm_pin;
}

#endif