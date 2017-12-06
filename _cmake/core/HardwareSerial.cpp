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
#include <string.h>

#include "wiring.h"

#include "HardwareSerial.h"

#ifdef MRAA_BACKEND

HardwareSerial::HardwareSerial(const char *_path)
{
	path = (char*)_path;
}

HardwareSerial::HardwareSerial(int _index)
{
	index = _index;
}

HardwareSerial::~HardwareSerial()
{
	if (dev != NULL)
		mraa_uart_stop(dev);
}

void HardwareSerial::begin(unsigned long baud)
{
	if (path != NULL) {
		dev = mraa_uart_init_raw(path);
	} else if (index != -1) {
		dev = mraa_uart_init(index);
	}
	mraa_uart_set_baudrate(dev, baud);
	mraa_uart_set_mode(dev, 8, MRAA_UART_PARITY_NONE, 1);
	mraa_uart_set_flowcontrol(dev, false, false);
}

void HardwareSerial::end() {}

int HardwareSerial::available(void) {
	return mraa_uart_data_available(dev, 0);
}

int HardwareSerial::peek(void) {
	// NOT YET IMPLEMENTED
	return 0;
}

int HardwareSerial::read(void) {
	char s;
	mraa_uart_read(dev, &s, 1);
	return s;
}

void HardwareSerial::flush(void) {
	mraa_uart_flush(dev);
}

size_t HardwareSerial::write(uint8_t c) {
	return mraa_uart_write(dev, (char*)&c, 1);
}

#else


fd_set HardwareSerial::tty_set;
int    HardwareSerial::max_fd=0;

// Constructors ////////////////////////////////////////////////////////////////

HardwareSerial::HardwareSerial(const char *path, bool getty)
{
	port_path = strdup(path);
	char *str = strdup(path);
	char *token;
	while ((token = strsep(&str, "/"))) {port_name = token;}
	free(str);
	buf_len=0;
	buf_loc=0;
	getty_console = getty;
	getty_console_disabled = false;
	pthread_mutex_init(&_mutex, 0);
}

void HardwareSerial::_detach_console( void )
{
	char buf[256];
	snprintf(buf, sizeof(buf),"systemctl stop getty@%s.service", port_name);
	system(buf);
	bzero(buf, strlen(buf));
	snprintf(buf, sizeof(buf),"systemctl stop serial-getty@%s.service", port_name);
	system(buf);
}

void HardwareSerial::_reattach_console( void )
{
	if (getty_console_disabled) {
		char buf[256];
		snprintf(buf, sizeof(buf),"systemctl start getty@%s.service", port_name);
		system(buf);
		bzero(buf, strlen(buf));
		snprintf(buf, sizeof(buf),"systemctl start serial-getty@%s.service", port_name);
		system(buf);
	}
}

void HardwareSerial::config_port(void)
{
	if (getty_console) {
		_detach_console();
		getty_console_disabled = true;
	}

	fd = open(port_path, O_RDWR | O_NOCTTY | O_NONBLOCK); 
	if(fd < 0)
	{
		perror(port_path);
		exit(-1);
	}

	if((fd+1) > max_fd) max_fd=fd+1; 	    /* find largest file descriptor */

	tcgetattr(fd, &old_tty_attr); 		    /* git attributes and store them in old_tty_attr */
	bzero(&new_tty_attr, sizeof(new_tty_attr)); /* zero out struct for new port settings 	     */

	/* 
	CS8     : Character Size 8bit Options(CS5, CS6, CS7)
		  1 stop bit (2 stop bits if CSTOPB is set)
	CLOCAL  : Ignore Modem Controle Lines
	CREAD   : Enable Receive
	*/
	new_tty_attr.c_cflag = CS8 | CLOCAL | CREAD;

	/*
	IGNPAR  : Ignore parity errors
	*/
	new_tty_attr.c_iflag = IGNPAR;

	/*
	OPOST	: Enable output processing
	*/
	new_tty_attr.c_oflag = OPOST;

	new_tty_attr.c_cc[VTIME]    = 0;     /* timeout before read */
	new_tty_attr.c_cc[VMIN]     = 1;     /* min number of characters before read */
	
}

void HardwareSerial::reset_port(void)
{
	/* 
	flush the line and apply settings for the old port
	then close the file
	*/
	tcflush(fd, TCIFLUSH);
	tcsetattr(fd, TCSANOW, &old_tty_attr);
	close(fd);
}

void * HardwareSerial::IRQHandler(void * pargs)
{
	HardwareSerial * HWSerial = (HardwareSerial*)pargs;
	unsigned int spin = 1;

	while (spin == 1) {
		HWSerial->readAndSave();
	}
	return pargs;
}


// Public Methods //////////////////////////////////////////////////////////////

void HardwareSerial::begin(unsigned long baud)
{	
	/*
	Baud Rates 14400 and 28800 are not supported by termbits.h
	*/

	config_port();
	switch(baud){
	case 300: baudrate=B300;
		break;
	case 600: baudrate=B600;
		break;
	case 1200: baudrate=B1200;
		break;
	case 4800: baudrate=B4800;
		break;
	case 9600: baudrate=B9600;
		break;
	case 19200: baudrate=B19200;
		break;
	case 38400: baudrate=B38400;
		break;
	case 57600: baudrate=B57600;
		break;
	case 115200: baudrate=B115200;
		break;
	case 230400: baudrate=B230400;
		break;
	case 460800: baudrate=B460800;
		break;
	case 500000: baudrate=B500000;
		break;
	case 576000: baudrate=B576000;
		break;
	case 921600: baudrate=B921600;
		break;
	case 1152000: baudrate=B1152000;
		break;
	case 1500000: baudrate=B1500000;
		break;
	case 2000000: baudrate=B2000000;
		break;
	case 2500000: baudrate=B2500000;
		break;
	case 3000000: baudrate=B3000000;
		break;
	case 3500000: baudrate=B3500000;
		break;
	case 4000000: baudrate=B4000000;
		break;
	default: baudrate=B115200;
		break;
	}
	/* Set Baud Rate */
	cfsetispeed(&new_tty_attr, baudrate);
	cfsetospeed(&new_tty_attr, baudrate);
	/* flush the line and apply settings for the new port */
	tcflush(fd, TCIFLUSH);
	tcsetattr(fd, TCSANOW, &new_tty_attr);

	// Initiate IRQ handler
	pthread_create(&_thread, NULL, &IRQHandler, this);
}

void HardwareSerial::end()
{
	reset_port();
}

int HardwareSerial::readAndSave(void)
{
	int temp_len;
	char temp_buf[CHAR_BUF_SIZE];

	/* zero */
	FD_ZERO(&tty_set);

	/*Add the file descriptor into the set*/
	FD_SET(fd, &tty_set);

	/*Check which files in the set are ready to read*/
	select(max_fd, &tty_set, NULL, NULL, 0);

	if(FD_ISSET(fd, &tty_set)) /* If fd is ready read contents into buf */
	{
		temp_len=::read(fd,temp_buf,CHAR_BUF_SIZE);
		pthread_mutex_lock(&_mutex);
		if (buf_len + temp_len < CHAR_BUF_SIZE) {
			memcpy(&buf[buf_len], temp_buf, temp_len);
			buf_len += temp_len;
		}
		pthread_mutex_unlock(&_mutex);
		return temp_len;
	}
	return 0;
}

int HardwareSerial::available(void)
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

int HardwareSerial::peek(void)
{
	if (!available())
		return -1;

	/* Return the last char in the buffer */
	return (int)buf[buf_loc];
}

int HardwareSerial::read(void)
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

void HardwareSerial::flush()
{
	/* Flush the line and reset buf_len and buf_loc */
	tcflush(fd, TCIFLUSH);
	buf_len=0;
	buf_loc=0;
}

size_t HardwareSerial::write(uint8_t c)
{	
	char val=(char)c;
	char *write_char=&val;
	int err=0;	

	/* Write char to the port */
	err=::write(fd, write_char, 1);
	if(err == 0) return -1;
	
	return 0;
}

#endif