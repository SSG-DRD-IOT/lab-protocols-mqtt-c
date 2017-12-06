#include <virtual_main.h>
#include <time.h>
#include <sched.h>
#include <signal.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <unistd.h>

static volatile int keepRunning = 1;
static pthread_t timerThread;

void *timerCloseHandler(void* arg __attribute__ ((unused))) {
	usleep(2000000);
	perror("shutting down forcefully");
	exit(-1);
}

void intHandler(int dummy __attribute__ ((unused))) {
	keepRunning = 0;
	// spawn a timeout thread to forcefully kill the process if locked into loop()
	pthread_create( &timerThread, NULL, timerCloseHandler, NULL);
	pthread_detach(timerThread);
}

int main(void)
{
	struct sched_param param;
	param.sched_priority = 20;

	if (sched_setscheduler(0, SCHED_RR, &param) < 0) {
		// try again, this time after adding this process to cpu cgroup
		char buf[256];
		sprintf(buf, "echo %d > /sys/fs/cgroup/cpu,cpuacct/tasks", getpid());
		system(buf);
		if (sched_setscheduler(0, SCHED_RR, &param) < 0) {
			perror("sched_setscheduler failed, timings will be wrong");
		}
	}

	signal(SIGINT, intHandler);

	clock_gettime(CLOCK_MONOTONIC, &prog_start_time);	
	init();
	setup();
	while (keepRunning) {
		loop();
		loop_connector();
		sched_yield();
	}

	pthread_cancel(timerThread);

	param.sched_priority = 0;
	if (sched_setscheduler(0, SCHED_IDLE, &param) != 0) {
		perror("sched_setscheduler failed, could not reset scheduler");
	}
	return 0;
}
