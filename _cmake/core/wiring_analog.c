/*
 * wiring_analog.c - analog input and output
 * Part of Arduino - http://www.arduino.cc/
 *
 * Copyright (c) 2013 Parav Nagarsheth
 * Copyright (c) 2005-2006 David A. Mellis
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

#include "wiring_analog.h"
#include "virtual_main.h"
#include "linux-virtual.h"
#include "sysfs.h"
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif


static uint32_t _writeResolution = 8;

// override at variant level if a different timebas eis needed
WEAK long analogGetPWMPeriod() {
	return 490;
}

#ifdef MRAA_BACKEND

int analogWrite(uint32_t pin, uint32_t value)
{
	mraa_pwm_context pwm;
	if (pin > mraa_get_pin_count()) {
		pwm = mraa_pwm_init(pin);
		if (pwm == NULL) {
			return -1;
		}
	} else {
		pwm = MRAAContexts[pin].pwm;
		if (pwm == NULL) {
			pwm = mraa_pwm_init(pin);
			MRAAContexts[pin].pwm = pwm;
		}
	}
	mraa_pwm_period_ms(pwm, 1000/analogGetPWMPeriod());
	mraa_pwm_write(pwm, (float)value/(float)(1 << _writeResolution));
	mraa_pwm_enable(pwm, 1);
	return value;
}

uint32_t analogRead(uint32_t pin)
{
	mraa_aio_context adc;
	if (pin > mraa_get_pin_count()) {
		adc = mraa_aio_init(pin);
		if (adc == NULL) {
			return 0;
		}
	} else {
		adc = MRAAContexts[pin].adc;
		if (adc == NULL) {
			adc = mraa_aio_init(pin);
			MRAAContexts[pin].adc = adc;
		}
	}
	return mraa_aio_read(adc);
}

#else

WEAK void analogWriteResolution(uint32_t res)
{
	_writeResolution = res;
}

static bool timerIrqActive = false;
static int pinsSwitchTick[MAX_PIN_INDEX] = {-1};
static int tickCounter = 0;

void changePinsState() {
	if (tickCounter == 0) {
		for (int i=0; i<MAX_PIN_INDEX; i++ ) {
			if (pinsSwitchTick[i] > 0) {
				digitalWrite(i, HIGH);
			}
		}
		tickCounter++;
		return;
	}
	for (int i=0; i < MAX_PIN_INDEX; i++ ) {
		if (pinsSwitchTick[i] == tickCounter) {
			digitalWrite(i, LOW);
		}
	}

	if (tickCounter > 500) {
		tickCounter = 0;
	} else {
		tickCounter++;
	}
}

WEAK int analogWrite(uint8_t pin, uint32_t value)
{

	int pinIndex = pin-BASE_PIN_INDEX;

	if (unlikely(pin > MAX_PIN_INDEX || pin < MIN_PIN_INDEX))
		return -1;

	if (unlikely(!(g_APinDescription[pinIndex].pinType & PWM))) {
		if (unlikely(!(g_APinDescription[pinIndex].pinType & GPIO))) {
			perror("not a pwm capable pin");
			return -1;
		}
		if (timerIrqActive == false) {
			attachContinuousTimerInterrupt(changePinsState, 1);
			timerIrqActive = true;
			for (int i=0; i < MAX_PIN_INDEX; i++ ) {
				pinsSwitchTick[i] = -1;
			}
		}
		value = value * 500;
		value = value >> _writeResolution;
		if (pinsSwitchTick[pinIndex] == -1) {
			pinMode(pinIndex, OUTPUT);
		}
		pinsSwitchTick[pinIndex] = value;
		return;
	}

	int pr;
	char prev[10];
	char buf[255];
	value = value * analogGetPWMPeriod();
	value = value >> _writeResolution;

	if (unlikely(g_APinDescription[pinIndex].state != PWM)) {
		muxSelect(pinIndex, PWM);
	}

	snprintf(buf, sizeof(buf), SYSFS_PWMN_PATH, (g_APinDescription[pinIndex].pinType >> PWM_N) & 0xF);

	int err = sysfs_read(buf, "enable", prev);
	char run[7];
	if (err < 0) {
		sysfs_read(buf, "run", prev);
		memcpy(run, "run\0", 4);
	} else {
		memcpy(run, "enable\0", 7);
	}
	pr = atoi(prev);

	if (pr == 0) {
		sysfs_write(buf, "period", analogGetPWMPeriod());
		sysfs_write(buf, "duty_cycle", value);
		sysfs_write(buf, run, 1);
	} else {
		sysfs_write(buf, "duty_cycle", value);
	}

	if (!value)
		sysfs_write(buf, run, 0);

	/* FIXME: Return 0 on success, negative on error, or similar */
	return pin;
}

WEAK uint32_t analogRead(uint32_t pin)
{

	int pinIndex = pin-BASE_PIN_INDEX;

	if (unlikely(pin > MAX_PIN_INDEX || pin < MIN_PIN_INDEX))
		return 0;

	if (unlikely(!(g_APinDescription[pinIndex].pinType & ADC))) {
		// try alternative pin (legacy) ( 0 -> A0 and so on )
		if (pin + A0 > MAX_PIN_INDEX || unlikely(!(g_APinDescription[pinIndex+A0].pinType & ADC))) {
			perror("not an adc capable pin");
			return 0;
		} else {
			pinIndex += A0;
		}
	}

	if (unlikely(g_APinDescription[pinIndex].state != ADC)) {
		muxSelect(pinIndex, ADC);
	}

	uint32_t adc_read = sysfs_adc_getvalue((g_APinDescription[pinIndex].pinType >> ADC_N) & 0xF);

	return adc_read;
}

#endif /*MRAA_BACKEND*/

#ifdef __cplusplus
}
#endif
