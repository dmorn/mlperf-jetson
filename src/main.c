// gcc preprocessor directive. Enables getopt from unistd.h
#define _POSIX_C_SOURCE 2

#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <time.h>

#include "api/submitter_implemented.h"
#include "api/internally_implemented.h"

int port;
char *line;
char *model_dir;

void
fatale(char *s) {
	fprintf(stderr, "\nerror: %s\n", s);
	exit(1);
}

void
fatalep(char *s) {
	perror(s);
	exit(1);
}

void
th_serialport_initialize(void) {
	struct termios tty;
	void cfmakeraw(struct termios *__termios_p);

	fprintf(stderr, "initializing serial port..");

	// Note the port is never closed.
	if((port = open(line, O_RDWR)) < 0) {
		fatalep("th_serialport_initialize");
	}
	if(tcgetattr(port, &tty) != 0) {
		fatalep("th_serialport_initialize");
	}
	cfmakeraw(&tty);
	cfsetispeed(&tty, B115200);
	cfsetospeed(&tty, B115200);

	if(tcsetattr(port, TCSANOW, &tty) != 0) {
		fatalep("th_serialport_initialize");
	}

	fprintf(stderr, ".done\n");
}

void
th_timestamp_initialize(void) {
	/* USER CODE 1 BEGIN */
	// Setting up BOTH perf and energy here
	/* USER CODE 1 END */
	/* This message must NOT be changed. */
	th_printf(EE_MSG_TIMESTAMP_MODE);
	/* Always call the timestamp on initialize so that the open-drain output
	is set to "1" (so that we catch a falling edge) */
	th_timestamp();
}

void th_pre() {}
void th_post() {}

void
th_command_ready(char volatile *p_command) {
	p_command = p_command;
	ee_serial_command_parser_callback((char *)p_command);
}

void
th_timestamp(void) {
	unsigned long microSeconds = 0ul;
	/* USER CODE 2 BEGIN */
	microSeconds = (unsigned long)((uint64_t)clock() * 1000000 / CLOCKS_PER_SEC);
	/* USER CODE 2 END */
	/* This message must NOT be changed. */
	th_printf(EE_MSG_TIMESTAMP, microSeconds);
}

int
usage(char *name) {
	fprintf(stderr, "usage: %s -d [model dir] -l [serial device, e.g. /dev/ttyPS0]\n", name);
	exit(2);
}

int
main(int argc, char *argv[]) {
	int opt;

	model_dir = "kws_ref_model";
	line = "/dev/ttyTHS1";
	while((opt = getopt(argc, argv, "d:l:")) != -1) {
		switch(opt) {
		case 'd':
			model_dir = optarg;
			break;
		case 'l':
			line = optarg;
			break;
		default:
			printf("opt=%c optarg=%s\n", opt, optarg);
			usage(*argv);
		}
	}

	fprintf(stderr, "line=%s model_dir=%s tf=%s\n", line, model_dir, TF_Version());

	ee_benchmark_initialize();
	while (1) {
		int c;
		c = th_getchar();
		fprintf(stderr, "char: [%c]\n", c);
		ee_serial_callback(c);
	}
	return 0;
}
