// gcc preprocessor directive. Enables getopt from unistd.h
#define _POSIX_C_SOURCE 2

#include <unistd.h>
#include <ctype.h>
#include <stddef.h>
#include <fcntl.h>
#include <time.h>
#include <cuda_profiler_api.h>

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
infer(uint8_t *buf, size_t bufn) {
	size_t n;
	float* res;

	tf_load(buf, bufn);
	tf_infer();
	res = tf_results();
	/*
	for(n = 0; n < kCategoryCount; n++) {
		fprintf(stdout, "%s\t: %.5f\n", kCategoryLabels[n], *res++);
	}
	*/
	tf_freetensors();
}

unsigned long
timestamp(void) {
	unsigned long microSeconds = 0ul;
	return (unsigned long)((uint64_t)clock() * 1000000 / CLOCKS_PER_SEC);
}

int
main(int argc, char *argv[]) {
	int opt, fd;
	size_t n, len, i;
	uint8_t *buf, *ptr;
	unsigned long tic, toc;

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

	if((buf = malloc(sizeof(uint8_t) * argc * kKwsInputSize)) == NULL)
		fatalep("malloc");
	ptr = buf;

	for(size_t i = argc; i > 0; i--) {
		fprintf(stdout, "loading MFSC from %s\n", *argv);
		if((fd = open(*argv++, O_RDONLY)) < 0)
			fatalep("open");

		len = sizeof(uint8_t) * kKwsInputSize;
		if((n = read(fd, ptr, len)) <= 0 || n < len)
			fatalep("read");

		close(fd);
		ptr+=len;
	}

	fprintf(stderr, "model_dir=%s tf=%s\n", model_dir, tf_version());
	tf_init();
	cudaProfilerStart();
	tic = timestamp();
	for(size_t i = argc; i > 0; i--) {
		fprintf(stdout, "inferring keywords in %s\n", *(argv-i));
		infer(buf+(i*kKwsInputSize), kKwsInputSize);
	}
	toc = timestamp();
	cudaProfilerStop();

	fprintf(stdout, "Inferences: %d\n", argc);
	fprintf(stdout, "Elapsed time (microseconds): %lu\n", toc - tic);
	fprintf(stdout, "Elapsed time (microseconds) per inference: %lu\n", (toc - tic)/argc);

	free(buf);
}
