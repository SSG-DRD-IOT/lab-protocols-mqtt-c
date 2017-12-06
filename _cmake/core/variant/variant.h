#ifndef _VARIANT_LINUX_
#define _VARIANT_LINUX_

#include "virtual_main.h"
#include <stdio.h>

#ifdef MRAA_BACKEND
#include "mraa.h"

#ifdef __cplusplus
#include "mraa.hpp"
#endif

/* HOWTO compile MRAA for generic target

cmake -DBUILDARCH=x86_64 -DBUILD_SHARED_LIBS=no -DCMAKE_CXX_FLAGS=-fPIC -DCMAKE_C_FLAGS=-fPIC -DJSONPLAT=no -DFIRMATA=yes -DBUILDSWIG=no -DBUILDSWIGNODE=no -DBUILDSWIGPYTHON=no -DBUILDTESTS=no..
cp src/libmraa.a $(core)/variant/$(variant_name)/$libs

*/

#endif

#define N_SPIDEV_PATH_0		0

#define PORT_TTY_NATIVE(n)  "/dev/ttyUSB"#n
#define PORT_TTY_CDC(n)     "/dev/ttyGS"#n

#endif
