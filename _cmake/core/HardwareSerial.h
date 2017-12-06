/*
  HardwareSerial.h - Hardware serial library for Wiring
  Copyright (c) 2006 Nicholas Zambetti.  All right reserved.

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
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

  Modified 28 September 2010 by Mark Sproul
  Modified 2  April	2014 by Cody Lacey
*/

#ifndef HardwareSerial_h
#define HardwareSerial_h

#include <inttypes.h>
#include <termios.h>
#include <sys/select.h>
#include "Stream.h"
#include <pthread.h>

#define CHAR_BUF_SIZE	255

#ifdef MRAA_BACKEND

#include "mraa.h"

class HardwareSerial : public Stream
{
  public:
    mraa_uart_context dev;
    HardwareSerial(const char* a);
    HardwareSerial(int index);
    ~HardwareSerial();
    void begin(unsigned long);
    void end(void);
    int readAndSave(void);
    virtual int available(void);
    virtual int peek(void);
    virtual int read(void);
    virtual void flush(void);
    virtual size_t write(uint8_t);
    using Print::write; // pull in write(str) and write(buf, size) from Print
    operator bool() { return dev != NULL; }
    char buf[CHAR_BUF_SIZE];
    char* path = NULL;
    int index = -1;
};

#else

class HardwareSerial : public Stream
{
  private:
    static fd_set tty_set;
    static int max_fd;
    struct timeval tty_ready_timeout;
    struct termios old_tty_attr,new_tty_attr;
    speed_t baudrate;
    int fd;
    volatile int buf_len;
    volatile int buf_loc;
    char buf[CHAR_BUF_SIZE];
    const char *port_path;
    char *port_name;
    bool getty_console;
    bool getty_console_disabled;
    void config_port(void);
    void reset_port(void);
    void _detach_console(void);
    void _reattach_console(void);

    // Use threads to support the model of available/peek the Serial class requires
	pthread_t _thread;
	pthread_mutex_t _mutex;
	pthread_barrier_t _barrier;
    
  public:
    HardwareSerial(const char* a, bool getty = false);
    ~HardwareSerial() {if (getty_console) {_reattach_console();};};

    static void * IRQHandler(void * pargs);	// static has no implicit this
    void begin(unsigned long);
    void end(void);
    int readAndSave(void);
    virtual int available(void);
    virtual int peek(void);
    virtual int read(void);
    virtual void flush(void);
    virtual size_t write(uint8_t);
    using Print::write; // pull in write(str) and write(buf, size) from Print
    operator bool() { return true; }
};

#endif

extern HardwareSerial Serial;
extern HardwareSerial Serial1;
extern HardwareSerial Serial2;
extern HardwareSerial Serial3;
extern HardwareSerial Serial4;
extern HardwareSerial Serial5;
extern HardwareSerial Serial6;

extern HardwareSerial SerialUSB;
extern HardwareSerial SerialUSB1;

#endif

