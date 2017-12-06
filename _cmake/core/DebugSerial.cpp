/*
  HardwareSerial.cpp - Hardware serial library for Wiring
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
  
  Modified 23 November 2006 by David A. Mellis
  Modified 28 September 2010 by Mark Sproul
  Modified 2  April	2014 by Cody Lacey
*/

#include <unistd.h>
#include <fcntl.h>

#include "wiring.h"
#include "shell.h"

#include "DebugSerial.h"

fd_set DebugSerial_::tty_set;
int    DebugSerial_::max_fd=0;

// Constructors ////////////////////////////////////////////////////////////////

DebugSerial_::DebugSerial_(const char *path)
{
	port_path=path;
	buf_len=0;
	buf_loc=0;
	pthread_mutex_init(&_mutex, 0);
}

void DebugSerial_::_detach_console( void )
{
	system("dmesg -n 1");
	system("systemctl stop getty@*.service");
	system("systemctl stop serial-getty@*.service");
}

void DebugSerial_::_reattach_console( void )
{
	system("dmesg -n 7");
	system("systemctl start getty@*.service");
	system("systemctl start serial-getty@*.service");
}

bool DebugSerial_::config_port(void)
{
	String s = System.runShellCommand("ps -p " +  String(getpid()) + " -o tty | grep pts");
	s = "/dev/" + s;
	fd = open(s.c_str(), O_RDWR | O_NOCTTY);
	if(fd < 0)
	{
		return false;
	}
	return true;
}

void DebugSerial_::reset_port(void)
{
	/* 
	flush the line and apply settings for the old port
	then close the file
	*/
	tcflush(fd, TCIFLUSH);
	tcsetattr(fd, TCSANOW, &old_tty_attr);
	close(fd);
}

void * DebugSerial_::IRQHandler(void * pargs)
{
	DebugSerial_ * DebugSerial = (DebugSerial_*)pargs;
	unsigned int spin = 1;

	while (spin == 1) {
		DebugSerial->readAndSave();
	}
	return pargs;
}


// Public Methods //////////////////////////////////////////////////////////////

bool DebugSerial_::begin(__attribute__((unused)) unsigned long baud)
{	
	/*
	Baud Rates 14400 and 28800 are not supported by termbits.h
	*/
	bool ret = config_port();

	// Initiate IRQ handler
	pthread_create(&_thread, NULL, &IRQHandler, this);
	return ret;
}

void DebugSerial_::end()
{
	reset_port();
}

int DebugSerial_::readAndSave(void)
{
	int temp_len;
	char temp_buf[CHAR_BUF_SIZE];

	temp_len=::read(fd,temp_buf,CHAR_BUF_SIZE);
	pthread_mutex_lock(&_mutex);
	if (buf_len + temp_len < CHAR_BUF_SIZE) {
		memcpy(&buf[buf_len], temp_buf, temp_len);
		buf_len += temp_len;
	}
	pthread_mutex_unlock(&_mutex);
	return temp_len;
}

int DebugSerial_::available(void)
{
	int ret = -1;

	/* Check if the contents have been read from the buffer */
	if((buf_len-buf_loc)>=0)
	{
		/* Return the number of characters remaining in the buffer */
		ret = (buf_len-buf_loc);
	}
	return ret;
}

int DebugSerial_::peek(void)
{
	if (!available())
		return -1;

	/* Return the last char in the buffer */
	return (int)buf[buf_loc];
}

int DebugSerial_::read(void)
{
	pthread_mutex_lock(&_mutex);
	
	/* If buffer is empty */
	if(buf_len == 0) {
		pthread_mutex_unlock(&_mutex);
		return -1;
	}

	int buf_val;

	/* If the buffer contains characters */
	if(buf_loc < buf_len)
	{
		/* Copy the character into buffer value */
		buf_val=(int)buf[buf_loc];
		buf_loc++;
	}
	/* If all chars have been read clear buf_len and buf_loc */
	if(buf_loc == buf_len)
	{
		buf_loc=0;
		buf_len=0;
	}

	pthread_mutex_unlock(&_mutex);
	return buf_val;	
}

void DebugSerial_::flush()
{
	/* Flush the line and reset buf_len and buf_loc */
	tcflush(fd, TCIFLUSH);
	buf_len=0;
	buf_loc=0;
}

size_t DebugSerial_::write(uint8_t c)
{
	char val=(char)c;
	char *write_char=&val;
	int err=0;	

	/* Write char to the port */
	err=::write(fd, write_char, 1);
	if(err == 0) return -1;
	
	return 0;
}
