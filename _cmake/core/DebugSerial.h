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

#ifndef DebugSerial_h
#define DebugSerial_h

#include <inttypes.h>
#include <termios.h>
#include <sys/select.h>
#include "Stream.h"
#include <pthread.h>

#define CHAR_BUF_SIZE	255

class DebugSerial_ : public Stream
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
    bool config_port(void);
    void reset_port(void);
    void _detach_console(void);
    void _reattach_console(void);

    // Use threads to support the model of available/peek the Serial class requires
	pthread_t _thread;
	pthread_mutex_t _mutex;
	pthread_barrier_t _barrier;
    
  public:
    DebugSerial_(const char*);

    static void * IRQHandler(void * pargs);	// static has no implicit this
    bool begin(unsigned long);
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

extern DebugSerial_ DebugSerial;

#endif

