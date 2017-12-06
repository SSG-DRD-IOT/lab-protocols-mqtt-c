#pragma once

#include "avr/pgmspace.h"
#include "pins_arduino.h"
#include "linux-virtual.h"
#include "WCharacter.h"
#include <stdio.h>
#include "shell.h"

#ifdef __cplusplus
unsigned long pulseIn(uint8_t pin, uint8_t state, unsigned long timeout = 1000000L);
#endif