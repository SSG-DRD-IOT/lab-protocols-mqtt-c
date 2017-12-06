/*
  sysfs.h

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

#ifndef Sysfs_h
#define Sysfs_h
#include <inttypes.h>
#ifdef __cplusplus
extern "C"{
#endif

#define MAX_SYSFS_BUF 100

int sysfs_read(const char *path, const char *filename, char *value);
int sysfs_write(const char *path, const char *filename, int value);
int sysfs_gpio_getvalue(uint8_t pin);
uint32_t sysfs_adc_getvalue(uint32_t channel);
int gpio_export(uint32_t gpio_pin);
int gpio_unexport(uint32_t gpio_pin);
int gpio_setdirection(uint32_t gpio_pin, const char *direction);
int gpio_setedge(unsigned int gpio, const char *edge);
int pwm_export(uint32_t pwm_pin);
int pwm_unexport(uint32_t pwm_pin);

int gpio_fd_open(unsigned int gpio_pin);
int gpio_fd_close(int fd);

#ifdef __cplusplus
}
#endif
#endif /* Sysfs_h */
