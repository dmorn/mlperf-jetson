// gcc preprocessor directive. Enables getopt from unistd.h
#define _POSIX_C_SOURCE 2

#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <ctype.h>
#include <stdio.h>

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
	// https://www.cmrr.umn.edu/~strupp/serial.html#2_1
	if((port = open(line, O_RDWR | O_NOCTTY | O_NDELAY)) < 0)
		fatalep("th_serialport_initialize");
	// Blocking reads.
	fcntl(port, F_SETFL, 0);

	if(tcgetattr(port, &tty) != 0)
		fatalep("th_serialport_initialize");

	// Baud
	cfsetispeed(&tty, B115200);
	cfsetospeed(&tty, B115200);
	// 8N1
	tty.c_cflag &= ~(PARENB | CSTOPB | CSIZE);
	tty.c_cflag |= CS8;
	// Quoting docs: The c_cflag member contains two options that should
	// always be enabled, CLOCAL and CREAD.
	tty.c_cflag |= (CLOCAL | CREAD);
	// Raw i/o
	tty.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
	tty.c_oflag &= ~OPOST;
	
	if(tcsetattr(port, TCSAFLUSH, &tty) != 0)
		fatalep("th_serialport_initialize");

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
	line = "/dev/ttyGS0";
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

	fprintf(stderr, "line=%s model_dir=%s tf=%s\n", line, model_dir, tf_version());

	ee_benchmark_initialize();
	while (1) {
		int c;
		if(!isprint(c = th_getchar()) || c == '\0') {
			fprintf(stderr, "discarding [%c]\n", c);
			continue;
		}
		ee_serial_callback(c);
	}
	return 0;
}
