#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <time.h>
#include <tensorflow/c/c_api.h>

#include "api/submitter_implemented.h"
#include "api/internally_implemented.h"

#define kNumCols 10
#define kNumRows 49
#define kNumChannels 1
#define kKwsInputSize kNumCols * kNumRows * kNumChannels
#define kCategoryCount 12

int port;
char *line;
const char *model_dir;
const char *tags = "serve";

char* kCategoryLabels[kCategoryCount] = {
	"down", "go", "left", "no", "off", "on",
	"right", "stop", "up", "yes", "silence", "unknown"
};

void
fatale(char *s) {
	perror(s);
	exit(2);
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

void
th_serialport_initialize(void) {
	struct termios tty;
	void cfmakeraw(struct termios *__termios_p);

	// Note the port is never closed.
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

TF_Status *s;
TF_Session *sess;
TF_Graph* g;
TF_SessionOptions *opts;
TF_Output ndin[1], ndout[1];
TF_Tensor *tin[1], *tout[1];

void
th_final_initialize(void) {
	/* 
	inputs['input_1'] tensor_info:
	      dtype: DT_FLOAT
	      shape: (-1, 49, 10, 1)
	      name: serving_default_input_1:0
	The given SavedModel SignatureDef contains the following output(s):
	  outputs['dense'] tensor_info:
	      dtype: DT_FLOAT
	      shape: (-1, 12)
	      name: StatefulPartitionedCall:0
	Method name is: tensorflow/serving/predict
	*/

	g = TF_NewGraph();
	s = TF_NewStatus();
	opts = TF_NewSessionOptions();
	sess = TF_LoadSessionFromSavedModel(opts, NULL, model_dir, &tags, 1, g, NULL, s);
	if(TF_GetCode(s) != TF_OK) {
		fatale(TF_Message(s));
	}

	ndin[0] = (TF_Output){TF_GraphOperationByName(g, "serving_default_input_1:0"), 0}; 
	ndout[0] = (TF_Output){TF_GraphOperationByName(g, "StatefulPartitionedCall:0"), 0};
}

void
tf_dealloc(void *data, size_t len, void *arg) {
	// Buffer is not dynamically allocated hence does not need to be
	// freed.
}

// Implement this method to prepare for inference and preprocess inputs.
void
th_load_tensor() {
	int8_t bufin[kKwsInputSize];
	float buft[kKwsInputSize];
	float bufout[kCategoryCount];

	size_t n, len;
	int64_t dimsin[] = {kNumCols, kNumRows}, dimsout[] = {kCategoryCount};

	TF_Tensor *in;

	// expected input: 10x49 8b MFCC
	len = kKwsInputSize * sizeof(int8_t);
	n = ee_get_buffer(bufin, len);
	if(n != len)
		fatale("ee_get_buffer: short read");
	for(int i = 0; i < kKwsInputSize; i++) {
		buft[i] = (float)bufin[i];
	}

	len = kKwsInputSize * sizeof(float);
	in = TF_NewTensor(TF_FLOAT, dimsin, 2, buft, len, tf_dealloc, NULL);
	if(in == NULL)
		fatale("input tensor allocation failure");
	tin[0] = (TF_Tensor*){in};

	/* I believe I do not have to do anything with it, TF_SessionRun will
	 * allocate memory for the tensors which I must free upon usage.
	len = kCategoryCount * sizeof(float);
	out = TF_NewTensor(TF_FLOAT, dimsout, 1, bufout, len, tf_dealloc, NULL);
	if(in == NULL)
		fatale("output tensor allocation failure");
	tout = {out};
	*/
}

// Implement this method with the logic to perform one inference cycle.
void
th_infer() {
	TF_SessionRun(
		sess, NULL,
		ndin, tin, 1,
		ndout, tout, 1,
		NULL, 0,
		NULL,
		s
	);
	if(TF_GetCode(s) != TF_OK) {
		fatale(TF_Message(s));
	}
}

void
th_post() {
	TF_DeleteGraph(g);
	TF_DeleteSessionOptions(opts);

	TF_CloseSession(sess, s);
	if(TF_GetCode(s) != TF_OK) {
		fatale(TF_Message(s));
	}
	TF_DeleteSession(sess, s);
	if(TF_GetCode(s) != TF_OK) {
		fatale(TF_Message(s));
	}

	/* TODO: delete tensors
	TF_DeleteTensor(tin); */
}

void
th_pre() {
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
	static char buf[1];
	if(read(port, buf, sizeof(buf)) < 0) {
		fatale("th_getchar");
	}
	// printf("debug: th_getchar: %c\n", buf[0]);
	return buf[0];
}

int
usage(char *name) {
	fprintf(stderr, "usage: %s [model dir] [serial device, e.g. /dev/ttyPS0]", name);
	exit(2);
}

int
main(int argc, char *argv[]) {
	if(argc < 2) {
		usage(argv[0]);
	}
	model_dir = argv[1];
	line = argv[2];

	printf("Hello from TensorFlow C library version %s\n", TF_Version());

	ee_benchmark_initialize();
	while (1) {
		int c;
		c = th_getchar();
		ee_serial_callback(c);
	}
	return 0;
}
