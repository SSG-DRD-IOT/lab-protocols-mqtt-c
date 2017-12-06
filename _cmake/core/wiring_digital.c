/*
  wiring_digital.c - digital input and output functions
  Part of Arduino - http://www.arduino.cc/

  Copyright (c) 2013 Parav Nagarsheth
  Copyright (c) 2005-2006 David A. Mellis

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

  Modified 28 September 2010 by Mark Sproul

*/

#include "wiring_digital.h"
#include "linux-virtual.h"
#include "virtual_main.h"
#include "Mux.h"
#include "pins_arduino.h"

#ifdef MRAA_BACKEND

void pinMode(uint32_t pin, uint8_t mode) {
	mraa_gpio_context gpio;
	if (pin > mraa_get_pin_count()) {
		gpio = mraa_gpio_init(pin);
		if (gpio == NULL) {
			return;
		}
	} else {
		gpio = MRAAContexts[pin].gpio;
		if (gpio == NULL) {
			// save status for late initialization
			MRAAContexts[pin].delayed_mode = mode;
			return;
		}
	}
	switch (mode) {
		case INPUT_PULLUP:
			mraa_gpio_mode(gpio, MRAA_GPIO_PULLUP);
			// no break is intentional
		case INPUT:
			mraa_gpio_dir(gpio, MRAA_GPIO_IN);
			break;
		case OUTPUT:
			mraa_gpio_dir(gpio, MRAA_GPIO_OUT);
			break;
	}
}

void digitalWrite(uint32_t pin, uint8_t val) {
	mraa_gpio_context gpio;
	if (pin > mraa_get_pin_count()) {
		gpio = mraa_gpio_init(pin);
		if (gpio == NULL) {
			return;
		}
	} else {
		gpio = MRAAContexts[pin].gpio;
		if (gpio == NULL) {
			gpio = mraa_gpio_init(pin);
			MRAAContexts[pin].gpio = gpio;
			pinMode(pin, MRAAContexts[pin].delayed_mode);
		}
	}
	mraa_gpio_write(gpio, val);
}

int digitalRead(uint32_t pin) {
	mraa_gpio_context gpio;
	if (pin > mraa_get_pin_count()) {
		gpio = mraa_gpio_init(pin);
		if (gpio == NULL) {
			return -1;
		}
	} else {
		gpio = MRAAContexts[pin].gpio;
		if (gpio == NULL) {
			gpio = mraa_gpio_init(pin);
			MRAAContexts[pin].gpio = gpio;
			pinMode(pin, MRAAContexts[pin].delayed_mode);
		}
	}
	return mraa_gpio_read(gpio);
}

#else

#define MAX_BUF 100

WEAK void pinMode(uint8_t pin, uint8_t mode)
{
	int pinIndex = pin-BASE_PIN_INDEX;

	if (pin > MAX_PIN_INDEX || pin < MIN_PIN_INDEX || !(g_APinDescription[pinIndex].pinType & GPIO))
  		return;

	if (unlikely(g_APinDescription[pinIndex].state != GPIO)) {
		// the pin in GPIO capable but not configured as GPIO

		if (g_APinDescription[pinIndex].state < 0) {
			// the pin is locked, report an error
			perror("pin locked");
			return;
		}
		muxSelect(pinIndex, GPIO);
	}

	if (mode == INPUT || mode == INPUT_PULLUP)
		gpio_setdirection(g_APinDescription[pinIndex].gpioPin, "in");
	else
		gpio_setdirection(g_APinDescription[pinIndex].gpioPin, "out");

	return;
}

WEAK void digitalWrite(uint8_t pin, uint8_t val)
{
	char buf[MAX_BUF];
	int pinIndex = pin-BASE_PIN_INDEX;

	if ((pin > MAX_PIN_INDEX || pin < MIN_PIN_INDEX) || \
		!((g_APinDescription[pinIndex].pinType & GPIO) || (g_APinDescription[pinIndex].pinType & LED)))
  		return;

	if (unlikely(g_APinDescription[pinIndex].state == PWM)) {
		muxSelect(pinIndex, GPIO);
	}

	if (unlikely(g_APinDescription[pinIndex].pinType & LED))
	{
		snprintf(buf, sizeof(buf), g_APinDescription[pinIndex].extraString, g_APinDescription[pinIndex].gpioPin);
		sysfs_write(buf, "brightness", val);
	} else {
		snprintf(buf, sizeof(buf), SYSFS_GPION_PATH, g_APinDescription[pinIndex].gpioPin);
		sysfs_write(buf, "value", val);
	}
}

WEAK int digitalRead(uint8_t pin)
{
	char buf[MAX_BUF], value[4];
	int pinIndex = pin-BASE_PIN_INDEX;

	if (pin > MAX_PIN_INDEX || pin < MIN_PIN_INDEX || !(g_APinDescription[pinIndex].pinType & GPIO))
		return -1;

	if (unlikely(g_APinDescription[pinIndex].state == PWM)) {
		muxSelect(pinIndex, GPIO);
	}

	if (g_APinDescription[pinIndex].pinType & GPIO)
	{
		snprintf(buf, sizeof(buf), SYSFS_GPION_PATH, g_APinDescription[pinIndex].gpioPin); 
		sysfs_read(buf, "value", value);
		return atoi(value);
	}
	else
		return 0;
}

#endif
