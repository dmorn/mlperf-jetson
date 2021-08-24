// gcc preprocessor directive. Enables getopt from unistd.h
#define _POSIX_C_SOURCE 2

#include <unistd.h>
#include <ctype.h>
#include <stddef.h>
#include <fcntl.h>

#include "api/submitter_implemented.h"

char* kCategoryLabels[kCategoryCount] = {
	"down", "go", "left", "no", "off", "on",
	"right", "stop", "up", "yes", "silence", "unknown"
};

int port = 1;
char *model_dir; 

int
usage(char *name) {
	fprintf(stderr, "usage: %s -d [model dir] [list of inferrable audio files, 490B each] \n", name);
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

void
infer(char *path) {
	int fd;
	size_t n, len;
	uint8_t buf[kKwsInputSize];
	float* res;

	if((fd = open(path, O_RDONLY)) < 0)
		fatalep("infer");

	len = sizeof(buf);
	if((n = read(fd, &buf, len)) <= 0 || n < len)
		fatalep("infer");

	fprintf(stdout, "spotting keywork in %s\n", path);
	tf_load(buf, kKwsInputSize);
	tf_infer();
	res = tf_results();
	for(n = 0; n < kCategoryCount; n++) {
		fprintf(stdout, "%s\t: %.5f\n", kCategoryLabels[n], *res++);
	}
	tf_freetensors();
	close(fd);
}

int
main(int argc, char *argv[]) {
	int opt;
	size_t n, len;

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
	argc -= optind;
	argv += optind;

	fprintf(stderr, "model_dir=%s tf=%s\n", model_dir, tf_version());
	tf_init();
	for(;argc > 0; argc--) {
		infer(*argv++);
	}
}
