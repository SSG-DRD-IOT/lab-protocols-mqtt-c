#include "variant.h"
#include <wordexp.h>
#include <unistd.h>
#include "virtual_main.h"
#include "HardwareSerial.h"

//stub empty functions and structs
HardwareSerial Serial("/dev/ttyS0");
HardwareSerial Serial1("/dev/ttyS1");
HardwareSerial Serial2("/dev/ttyS2");
HardwareSerial Serial3("/dev/ttyS3");
HardwareSerial Serial4("/dev/ttyS4");
HardwareSerial SerialUSB(PORT_TTY_CDC(0));
HardwareSerial SerialUSB1(PORT_TTY_NATIVE(0));
DebugSerial_ DebugSerial("/dev/pts/11");

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_ALLOWED_GPIONUMBER	300

mraa_platform_t g_platform;
MRAAPinDescription MRAAContexts[MAX_ALLOWED_GPIONUMBER] = {NULL, NULL, NULL, -1};

WEAK void init_connector() {}
WEAK void loop_connector() {}

void init() {
	if (mraa_init() != MRAA_SUCCESS) {
		exit(1);
	}
#ifndef PLATFORM_PINMUXING_BUG
	g_platform = mraa_get_platform_type();
	for (uint32_t i=0; i < mraa_get_pin_count(); i++) {
    	mraa_aio_context y;
    	y = mraa_aio_init(i);
    	if (y != NULL) {
    		MRAAContexts[i].adc = y;
    	}

    	mraa_pwm_context z;
    	z = mraa_pwm_init(i);
    	if (z != NULL) {
    		MRAAContexts[i].pwm = z;
    	}

        // workaround: export GPIO pins only if they're not ADC or PWM capable
        if (y == NULL && z == NULL) {
            mraa_gpio_context x;
            x = mraa_gpio_init(i);
            if (x != NULL) {
                MRAAContexts[i].gpio = x;
            }
        }
	}
#endif
    init_connector();
}

void muxSelect() {


}

#ifdef __cplusplus
}
#endif
