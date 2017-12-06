#include "shell.h"
#include "Arduino.h"

unsigned int Process::runShellCommand(const String& command) {
  begin(command);
  return run();
}

void Process::runShellCommandAsynchronously(const String &command) {
  begin(command);
  runAsynchronously();
  //close();
}

size_t Process::write(uint8_t c) {
  fputc(c, fp_out);
  return 1;
}

void Process::flush() {
}

int Process::available() {
  // Look if there is new data available
  running();
  return buffered;
}

int Process::read() {
  running();
  if (buffered == 0)
    return -1; // no chars available
  else {
    buffered--;
    return buffer[readPos++];
  }
}

int Process::peek() {
  running();
  if (buffered == 0)
    return -1; // no chars available
  else
    return buffer[readPos];
}

void Process::doBuffer() {

  // If there are already char in buffer exit
  if (buffered > 0)
    return;

  readPos = 0;

  // Try to buffer up to 32 characters
  char temp[512];
  /* Read the output a line at a time - output it. */
  if (fgets(temp, sizeof(temp)-1, fp) != NULL) {
    memcpy(&buffer[buffered], temp, strlen(temp));
    buffered += strlen(temp);
  }
}

void Process::begin(const String &command) {
  started = false;
  //fp = NULL;
  cmdline = new String(command);
}

void Process::addParameter(const String &param) {
  *cmdline += " ";
  *cmdline += param;
}

void Process::runAsynchronously() {
  /* Open the command for reading. */
  pid = popen2(*cmdline, fp, fp_out);
  if (fp == NULL) {
    printf("Failed to run command\n" );
    return;
  }
  int d = fileno(fp);
  int flags;
  flags = fcntl(d, F_GETFL, 0);
  flags |= O_NONBLOCK;
  fcntl(d, F_SETFL, flags);
  d = fileno(fp_out);
  flags = fcntl(d, F_GETFL, 0);
  flags |= O_NONBLOCK;
  fcntl(d, F_SETFL, flags);
  started = true;
}

bool Process::running() {

  int stat = 0;
  pid_t wpid = 0;
  bool ret;
  if (fp != NULL) {
    doBuffer();
  }
  wpid = waitpid(pid, &stat, WNOHANG|WUNTRACED);
  if (wpid == 0 || wpid == pid)
    return true;
  if (wpid == -1 && errno != EINTR)
    return false;
  ret = (WIFEXITED(stat) == 0);
  return (ret);
}

unsigned int Process::exitValue() {
  while (running()) {
    delay(100);
  }
  close();
  return WEXITSTATUS(exitval);
}

unsigned int Process::run() {
  runAsynchronously();
  while (running())
    delay(100);
  return exitValue();
}

void Process::end() {
  /* close */
  kill(pid, SIGTERM);
  close();
}

void Process::close() {
  /* close */
  exitval = pclose2(fp, fp_out, pid);
  started = false;
  fp = NULL;
  delete cmdline;
}

#include <stdio.h>
#include <string.h>    //strlen
#include <stdlib.h>    //strlen
#include <sys/socket.h>
#include <arpa/inet.h> //inet_addr
#include <unistd.h>    //write
#include <pthread.h> //for threading , link with lpthread

void* ConsoleClass::connection_handler(void* arg)
{
    struct sockaddr_in client;
    ConsoleClass* console = (ConsoleClass*)arg;
    while (console->new_socket = accept(console->socket_desc, (struct sockaddr *)&client, (socklen_t*)&(console->c))) {
        console->client_connected = true;
        memset(console->buffer, 0, sizeof(console->buffer));
        //while (1) {
        //  console->doBuffer();
        //}
    }
    console->client_connected = false;

    return 0;
}

int ConsoleClass::begin() {
    struct sockaddr_in server;

    system("killall telnet");

    //Create socket
    socket_desc = socket(AF_INET , SOCK_STREAM , 0);
    if (socket_desc == -1)
    {
        printf("Could not create socket");
    }

    //Prepare the sockaddr_in structure
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons( 6571 );

    // Put the socket in non-blocking mode:
    //if(fcntl(socket_desc, F_SETFL, fcntl(socket_desc, F_GETFL) | O_NONBLOCK) < 0) {
    //    puts("set non blocking failed");
    //}

    int enable = 1;
    while (setsockopt(socket_desc, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) {
        puts("setopt failed");
    }

    //Bind
    while ( bind(socket_desc,(struct sockaddr *)&server , sizeof(server)) < 0)
    {
        delay(100);
        puts("bind failed");
    }
    puts("bind done");

    //Listen
    listen(socket_desc , 3);

    c = sizeof(struct sockaddr_in);

    memset(buffer, 0, sizeof(buffer));

    if( pthread_create( &sniffer_thread , NULL ,  &ConsoleClass::connection_handler , this) < 0)
    {
        perror("could not create thread");
        return 0;
    }

    return 1;
}

void ConsoleClass::doBuffer() {

  // If there are already char in buffer exit
  if (buffered > 0)
    return;

  readPos = 0;
  memset(buffer, 0, sizeof(buffer));

  // Try to buffer up to 32 characters
  char temp[512];
  memset(temp, 0, sizeof(temp));
  /* Read the output a line at a time - output it. */
  if (recv(new_socket, temp, sizeof(temp)-1, MSG_DONTWAIT | MSG_NOSIGNAL) != 1) {
    memcpy(&buffer[buffered], temp, strlen(temp));
    buffered += strlen(temp);
  }
}

int ConsoleClass::available() {
  // Look if there is new data available
  doBuffer();
  return buffered;
}

int ConsoleClass::read() {
  doBuffer();
  if (buffered == 0)
    return -1; // no chars available
  else {
    buffered--;
    return buffer[readPos++];
  }
}

int ConsoleClass::peek() {
  doBuffer();
  if (buffered == 0)
    return -1; // no chars available
  else
    return buffer[readPos];
}

size_t ConsoleClass::write(const char *buffer, size_t size) {
  for (size_t i = 0; i< size; i++)
    write(buffer[i]);
  return size;
}

size_t ConsoleClass::write(uint8_t character) {
  return send(new_socket , &character , 1, MSG_NOSIGNAL);
}

/* System subclass - utilities */

int SystemClass::setGovernor(char* governor, int cpu) {
  char cmd[1024];
  snprintf(cmd, sizeof(cmd), "echo %s > /sys/devices/system/cpu/cpu%d/cpufreq/scaling_governor", governor, cpu);
  return system(cmd);
}

void SystemClass::goPowerSave() {
  char msg[] = "powersave";
  setGovernor(msg, 0);
}

void SystemClass::goPerformance() {
  char msg[] = "performance";
  setGovernor(msg, 0);
}

void SystemClass::goOndemand() {
  char msg[] = "ondemand";
  setGovernor(msg, 0);
}

String SystemClass::getTime() {
  Process p;
  String ret = "";
  p.runShellCommand("date +\%H:\%M:\%S");
  while (p.available()) {
    ret += (char)(p.read());
  }
  ret.trim();
  return ret;
}

String SystemClass::getDate() {
  Process p;
  String ret = "";
  p.runShellCommand("date +\%d/\%m/\%y");

  while (p.available()) {
    ret += (char)(p.read());
  }
  ret.trim();
  return ret;
}

String SystemClass::runShellCommand(String s) {
  Process p;
  String ret = "";
  p.runShellCommand(s);

  while (p.available()) {
    ret += (char)(p.read());
  }
  ret.trim();
  return ret;
}

SystemClass System;
ConsoleClass Console;
