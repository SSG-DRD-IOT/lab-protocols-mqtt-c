/*
 * twi.h
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

#ifndef twi_h
#define twi_h

#include <inttypes.h>
#ifdef __cplusplus
extern "C"{
#endif

int twi_getadapter(uint32_t twi_bus_address);
int twi_openadapter(char* _twi_adapter);
int twi_setslave(int twi_fd, uint8_t addr);
int twi_writebyte(int twi_fd, uint8_t byte);
int twi_writebytes(int twi_fd, uint8_t *bytes, uint8_t length);
int twi_readbyte(int twi_fd);
int twi_readbytes(int twi_fd, uint8_t *buf, int length);
int twi_readwrite(int twi_fd);
int twi_add_to_buf(uint8_t addr, uint8_t rw, uint8_t *value, int length);

#ifdef __cplusplus
}
#endif
#endif /* twi_h */
