#ifndef virtual_main_h
#define virtual_main_h

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <inttypes.h>
#include "sysfs.h"
#include "variant.h"

#ifdef __cplusplus
extern "C"{
#endif


#ifdef MRAA_BACKEND
#include "mraa.h"

typedef struct _MRAAPinDescription {
	mraa_gpio_context gpio;
	mraa_aio_context adc;
	mraa_pwm_context pwm;
	int8_t delayed_mode;
} MRAAPinDescription;

extern MRAAPinDescription MRAAContexts[];
#endif

/* Definition and types for pins */
typedef enum _PinTypes {
	GPIO 		= 1 << 0,
	PWM 		= 1 << 1,
	_SPI 		= 1 << 2,
	UART 		= 1 << 3,
	I2C 		= 1 << 4,
	LED 		= 1 << 5,
	ADC 		= 1 << 6,
	GPIO_N		= 8,
	PWM_N		= 12,
	_SPI_N		= 16,
	UART_N		= 20,
	I2C_N		= 24,
	ADC_N		= 28
} PinTypes;

#define _SPI(n)	(_SPI 	| n << _SPI_N)
#define I2C(n)	(I2C 	| n << I2C_N)
#define PWM(n)	(PWM 	| n << PWM_N)
#define UART(n)	(UART 	| n << UART_N)
#define ADC(n)	(ADC 	| n << ADC_N)
#define GPIO(n)	(GPIO 	| n << GPIO_N)

#define MAX_BUF			100
#define byte			uint8_t
#define WEAK 	__attribute__((weak))

/* Types used for the tables below */
typedef struct _PinDescription {
	uint32_t headerPin;
	uint32_t gpioPin;
	uint32_t pinType;
	int32_t state;
	char* extraString;
} PinDescription;

/* Pins table to be instanciated into variant.cpp */
extern PinDescription g_APinDescription[] ;

#define digitalPinToInterrupt(pin) 		pin
#define microsecondsToClockCycles(a) 	( ((a) * (F_CPU / 1000L)) / 1000L )

/* Program start time for millis and micros. Instantiated in main.cpp */
extern struct timespec prog_start_time;
#ifdef __cplusplus
}
#include "HardwareSerial.h"
#include "DebugSerial.h"
#include "WMath.h"
#endif


#include "wiring.h"
#include "wiring_digital.h"
#include "wiring_analog.h"
#endif

