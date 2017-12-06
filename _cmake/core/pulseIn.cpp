/*
pulseIn.cpp pulseIn library for Intel Galileo Gen 2
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

/* 
 * Author:
 * Dino Tinitigan <dino.tinitigan@intel.com>
 *
 * Port of pulseIn Library for Galileo and Galileo 2.0
 *
*/

#include <Arduino.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>

static unsigned long volatile timeout_thd;

void *timeoutHandler(void *arg __attribute__ ((unused))) {
  usleep(timeout_thd);
  timeout_thd = 0;
  return 0;
}

WEAK unsigned long pulseIn(uint8_t pin, uint8_t state, unsigned long timeout)
{
  timeout_thd = timeout;
  pthread_t timeoutThread;
  pthread_create( &timeoutThread, NULL, timeoutHandler, NULL);
  pthread_detach(timeoutThread);

  // wait for any previous pulse to end
  while (digitalRead(pin) == state) {
    usleep(10);
    if (timeout_thd == 0) {
      return 0;
    }
  }

  // wait for the pulse to start
  while (digitalRead(pin) != state) {
    usleep(10);
    if (timeout_thd == 0) {
      return 0;
    }
  }

  unsigned long start = micros();
  // wait for the pulse to stop
  while (digitalRead(pin) == state) {
    usleep(1);
    if (timeout_thd == 0) {
      return 0;
    }
  }

  pthread_cancel(timeoutThread);
  return micros() - start;
}