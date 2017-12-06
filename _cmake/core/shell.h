#include "WString.h"
#include "Stream.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <sys/wait.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>

#include "Arduino.h"

#ifdef __cplusplus

#ifndef _SHELL_H_
#define _SHELL_H_

pid_t popen2(String command, FILE* & fd_in, FILE* &fd_out );
int pclose2(FILE * fp, FILE * fp_out, pid_t pid);

class SystemClass {

public:
	SystemClass() {};
	void goPowerSave();
	void goPerformance();
	void goOndemand();
	int setGovernor(char* governor, int cpu = 0);
	String getDate();
	String getTime();
    String runShellCommand(String s);
};


class ConsoleClass : public Stream {
  public:
    ConsoleClass() : client_connected(false) {};
    ~ConsoleClass()  {end();};

    int begin();
    void end() {pthread_cancel(sniffer_thread);};

    bool connected() {return client_connected == true;};

    // Stream methods
    // (read from console socket)
    int available();
    int read();
    int peek();
    // (write to console socket)
    size_t write(uint8_t);
    size_t write(const char *buffer, size_t size);
    void flush() {};

    static void* connection_handler(void*);

    operator bool () {
      return connected();
    }

  private:
    void doBuffer();
    volatile uint8_t buffered;
    volatile uint8_t readPos;
    static const int BUFFER_SIZE = 10000;
    uint8_t buffer[BUFFER_SIZE];
    int socket_desc;
    int c;
    pthread_t sniffer_thread;
    volatile int new_socket;
    int *new_sock;

    bool client_connected;
};


class Process : public Stream {

  public:
    // Constructor with a user provided BridgeClass instance
    Process() :
      started(false), buffered(0), readPos(0) { }

    void begin(const String &command);
    void addParameter(const String &param);
    unsigned int run();
    void runAsynchronously();
    bool running();
    unsigned int exitValue();
    void close();
    void end();

    unsigned int runShellCommand(const String &command);
    void runShellCommandAsynchronously(const String &command);

    operator bool () {
      return started;
    }

    // Stream methods
    // (read from process stdout)
    int available();
    int read();
    int peek();
    // (write to process stdin)
    size_t write(uint8_t);
    void flush();
    // TODO: add optimized function for block write

  private:
    uint8_t handle;
    String *cmdline;
    bool started;
    FILE *fp;
    FILE *fp_out;
    int exitval;
    int pid;

    void doBuffer();
    uint8_t buffered;
    uint8_t readPos;
    static const int BUFFER_SIZE = 1024;
    uint8_t buffer[BUFFER_SIZE];
};

extern ConsoleClass Console;
extern SystemClass System;

#endif
#endif