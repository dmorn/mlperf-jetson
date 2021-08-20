// gcc preprocessor directive. Enables getopt from unistd.h
#define _POSIX_C_SOURCE 2

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

#define kSerialBufSize 256
int sfd;

char *line;
char *model_dir;
const char *tags = "serve";

char* kCategoryLabels[kCategoryCount] = {
	"down", "go", "left", "no", "off", "on",
	"right", "stop", "up", "yes", "silence", "unknown"
};

static TF_Status *s;
static TF_Session *sess;
static TF_Graph* g;
static TF_SessionOptions *opts;

// Sizes are model specific.
static TF_Output ndi[1], ndo[1];
static TF_Tensor *tfi[1], *tfo[1];

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
tf_load(int8_t *buf, size_t n) {
	size_t len = n * sizeof(buf);
	int64_t dimsi[] = {1, kNumRows, kNumCols, 1};
	int64_t dimso[] = {1, kCategoryCount};

	// Input tensor
	*tfi = TF_AllocateTensor(TF_FLOAT, dimsi, 4, len);
	if(*tfi == NULL)
		fatale("input tensor allocation failure");

	for(float *p = (float*) TF_TensorData(*tfi); n > 0; n--) {
		*p++ = (float) (*(buf++));
	}

	// Output tensor
	len = kCategoryCount * sizeof(float);
	*tfo = TF_AllocateTensor(TF_FLOAT, dimso, 2, len);
	if(*tfo == NULL)
		fatale("output tensor allocation failure");
}

void
tf_free() {
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

	TF_DeleteTensor(*tfi);
	TF_DeleteTensor(*tfo);
}

void
th_serialport_initialize(void) {
	struct termios tty;
	void cfmakeraw(struct termios *__termios_p);

	fprintf(stderr, "initializing serial port..");

	// Note the port is never closed.
	if((sfd = open(line, O_RDWR)) < 0) {
		fatalep("th_serialport_initialize");
	}
	if(tcgetattr(sfd, &tty) != 0) {
		fatalep("th_serialport_initialize");
	}
	cfmakeraw(&tty);
	cfsetispeed(&tty, B115200);
	cfsetospeed(&tty, B115200);

	if(tcsetattr(sfd, TCSANOW, &tty) != 0) {
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
	fprintf(stderr, "initializing tensors..");

	g = TF_NewGraph();
	s = TF_NewStatus();
	opts = TF_NewSessionOptions();
	sess = TF_LoadSessionFromSavedModel(opts, NULL, model_dir, &tags, 1, g, NULL, s);
	if(TF_GetCode(s) != TF_OK) {
		fatale(TF_Message(s));
	}

	if((ndi->oper = TF_GraphOperationByName(g, "serving_default_input_1")) == NULL)
		fatale("input operation not found within graph");
	ndi->index = 0;

	if((ndo->oper = TF_GraphOperationByName(g, "StatefulPartitionedCall")) == NULL)
		fatale("output operation not found within graph");
	ndo->index = 0;

	fprintf(stderr, ".done\n");
	fprintf(stderr, "** READY\n");
}

void
th_post() {
}

// Add to this method to return real inference results.
void
th_results() {
	float *p;

	fprintf(stderr, "th_results called!\n");

	if(*tfo == NULL)
		fatale("output tensor is empty");
	if(TF_TensorElementCount(*tfo) != kCategoryCount)
		fatale("unexpected tensor element count");

	p = (float*)TF_TensorData(*tfo);

	th_printf("m-results-[");
	for (int i = 0; i < kCategoryCount; i++) {
		fprintf(stderr, "res(%d,%s) -> %.8f\n", i, kCategoryLabels[i], *p);
		th_printf("%f", *p++);
		if (i < (kCategoryCount - 1)) {
			th_printf(",");
		}
	}
	th_printf("]\r\n");

	// TODO: free up everything? Not if infer is called before a load.
}

// Implement this method to prepare for inference and preprocess inputs.
void
th_load_tensor() {
	static int nload = 0;
	int8_t buf[kKwsInputSize];
	size_t n, len;
	void tf_load(int8_t*, size_t);

	fprintf(stderr, "%d: th_load_tensor called!\n", nload++);

	// expected input: 10x49 8b MFCC
	len = kKwsInputSize * sizeof(int8_t);
	n = ee_get_buffer(buf, len);
	if(n != len)
		fatale("ee_get_buffer: short read");

	tf_load(buf, kKwsInputSize);
}

// Implement this method with the logic to perform one inference cycle.
void
th_infer() {
	static int ninfer = 0;

	fprintf(stderr, "%d: th_infer called!\n", ninfer++);
	TF_SessionRun(
		sess, NULL,
		ndi, tfi, 1,
		ndo, tfo, 1,
		NULL, 0,
		NULL,
		s
	);
	if(TF_GetCode(s) != TF_OK) {
		fatale(TF_Message(s));
	}
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
	// vdprintf(2, format, ap);
	return vdprintf(sfd, format, ap);
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
	static char buf[kSerialBufSize];
	static size_t idx = 0, n = 0;

	if(idx < n)
		return buf[idx++];

	if((n = read(sfd, buf, kSerialBufSize)) < 0) {
		fatale("th_getchar");
	}
	idx = 0;
	return buf[idx++];
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
