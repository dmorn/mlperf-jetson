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

#define kNumCols 10
#define kNumRows 49
#define kNumChannels 1
#define kKwsInputSize kNumCols * kNumRows * kNumChannels
#define kCategoryCount 12

char* kCategoryLabels[kCategoryCount] = {
	"down", "go", "left", "no", "off", "on",
	"right", "stop", "up", "yes", "silence", "unknown"
};

// Implement this method to prepare for inference and preprocess inputs.
void
th_load_tensor() {
}

// Add to this method to return real inference results.
void
th_results() {
	const int nresults = 3;
	/* The results need to be printed back in exactly this format; if
	 * easier to just modify this loop than copy to results[] above, do
	 * that.
	*/
	th_printf("m-results-[");
	for (size_t i = 0; i < kCategoryCount; i++) {
		float converted = 0;

		// Some platforms don't implement floating point formatting.
		th_printf("0.%d", (int)converted * 10);
		th_printf("%d", (int)converted * 100 % 10);
		th_printf("%d", (int)converted * 1000 % 10);
		if (i < (nresults - 1)) {
		th_printf(",");
		}
	}
	th_printf("]\r\n");
}

// Implement this method with the logic to perform one inference cycle.
void
th_infer() {
}

/// \brief optional API.
void
th_final_initialize(void) {
}

void
th_pre() {
}

void
th_post() {
}

void
th_command_ready(char volatile *p_command) {
	p_command = p_command;
	ee_serial_command_parser_callback((char *)p_command);
}

// th_libc implementations.
int
th_strncmp(const char *str1, const char *str2, size_t n) {
	return strncmp(str1, str2, n);
}

char*
th_strncpy(char *dest, const char *src, size_t n) {
	return strncpy(dest, src, n);
}

size_t
th_strnlen(const char *str, size_t maxlen) {
	size_t strnlen (const char *__string, size_t __maxlen);
	return strnlen(str, maxlen);
}

char*
th_strcat(char *dest, const char *src) {
	return strcat(dest, src);
}

char*
th_strtok(char *str1, const char *sep) {
	return strtok(str1, sep);
}

int
th_atoi(const char *str) {
	return atoi(str);
}

void* 
th_memset(void *b, int c, size_t len) {
	return memset(b, c, len);
}

void*
th_memcpy(void *dst, const void *src, size_t n) {
	return memcpy(dst, src, n);
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

void
fatale(char *s) {
	perror(s);
	exit(2);
}

int
th_vprintf(const char *format, va_list ap) {
	int vdprintf(int __fd, const char *__restrict __fmt, __gnuc_va_list __arg);
	return vdprintf(port, format, ap);
}

void
th_printf(const char *p_fmt, ...) {
	va_list args;
	va_start(args, p_fmt);
	(void)th_vprintf(p_fmt, args); /* ignore return */
	va_end(args);
}

char
th_getchar() {
	char buf[1];
	if(read(port, buf, sizeof(buf)) < 0) {
		fatale("th_getchar");
	}
	// printf("debug: th_getchar: %c\n", buf[0]);
	return buf[0];
}

void
th_serialport_initialize(void) {
	struct termios tty;
	void cfmakeraw(struct termios *__termios_p);

	// TODO: when is this resource released?
	if((port = open(line, O_RDWR)) < 0) {
		fatale("th_serialport_initialize");
	}
	if(tcgetattr(port, &tty) != 0) {
		fatale("th_serialport_initialize");
	}
	cfmakeraw(&tty);
	cfsetispeed(&tty, B115200);
	cfsetospeed(&tty, B115200);

	if(tcsetattr(port, TCSANOW, &tty) != 0) {
		fatale("th_serialport_initialize");
	}
}

int
usage(char *name) {
	fprintf(stderr, "usage: %s [serial device, e.g. /dev/ttyPS0]", name);
	exit(2);
}

int
main(int argc, char *argv[]) {
	if(argc < 1) {
		usage(argv[0]);
	}
	line = argv[1];

	ee_benchmark_initialize();
	while (1) {
		int c;
		c = th_getchar();
		ee_serial_callback(c);
	}
	return 0;
}
