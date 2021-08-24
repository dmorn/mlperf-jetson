// gcc preprocessor directive. Enables getopt from unistd.h
#define _POSIX_C_SOURCE 2

#include <unistd.h>
#include <ctype.h>
#include <stddef.h>

#include "api/submitter_implemented.h"

char* kCategoryLabels[kCategoryCount] = {
	"down", "go", "left", "no", "off", "on",
	"right", "stop", "up", "yes", "silence", "unknown"
};

int port = 1;
char *model_dir; 

int
usage(char *name) {
	fprintf(stderr, "usage: %s -d [model dir] <image.bin \n", name);
	exit(2);
}

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

int
main(int argc, char *argv[]) {
	int opt;
	size_t n, len;
	uint8_t buf[kKwsInputSize];
	float* res;

	model_dir = "kws_ref_model";
	while((opt = getopt(argc, argv, "d:")) != -1) {
		switch(opt) {
		case 'd':
			model_dir = optarg;
			break;
		default:
			printf("opt=%c optarg=%s\n", opt, optarg);
			usage(*argv);
		}
	}

	fprintf(stderr, "model_dir=%s tf=%s\n", model_dir, tf_version());

	len = sizeof(buf);
	if((n = read(0, &buf, len)) <= 0 || n < len)
		perror("short read on stdin");

	tf_init();
	tf_load(buf, kKwsInputSize);
	fprintf(stderr, "spotting keyword\n");
	tf_infer();
	res = tf_results();
	for(n = 0; n < kCategoryCount; n++) {
		fprintf(stdout, "%s\t: %.5f\n", kCategoryLabels[n], *res++);
	}
	tf_freetensors();
}
