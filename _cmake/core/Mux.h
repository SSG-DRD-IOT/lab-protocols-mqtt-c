/* Mux.c high level abstraction for muxing across boards
Copyright (C) 2014 Intel Corporation

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef __MUX_H__
#define __MUX_H__

#include <wiring_digital.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Compulsory functions - must be defined in variant files */
int muxInit(void);
int muxSelect(uint8_t pin, int32_t newFunction);

/* Wrapper functions for muxSelect() */
int muxSelectAnalogPin(uint8_t pin);
int muxSelectPwmPin(uint8_t pin);
int muxSelectUart(uint8_t interface);
int muxDeselectUart(uint8_t interface);
int muxSelectSpi(uint8_t interface);
int muxSelectI2c(uint8_t interface);

#ifdef __cplusplus
}
#endif

#endif /* __MUX_H__ */

