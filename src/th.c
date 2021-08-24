#include "api/internally_implemented.h"
#include "api/submitter_implemented.h"

void fatale(char*);

void
th_final_initialize(void) {
	tf_init();
}

void
th_load_tensor() {
	static int nload = 0;
	uint8_t buf[kKwsInputSize];
	size_t n, len;

	fprintf(stderr, "%d: th_load_tensor called!\n", nload++);

	// expected input: 10x49 8b MFCC
	len = sizeof(buf);
	n = ee_get_buffer(buf, sizeof(buf));
	if(n != len)
		fatale("ee_get_buffer: short read");

	tf_load(buf, kKwsInputSize);
}

void
th_infer(void) {
	static int ninfer = 0;

	fprintf(stderr, "%d: th_infer called!\n", ninfer++);
	tf_infer();
}

void
th_results(void) {
	float* p;

	fprintf(stderr, "th_results called!\n");
	p = tf_results();
	th_printf("m-results-[");
	for (int i = 0; i < kCategoryCount; i++) {
		th_printf("%f", *p++);
		if (i < (kCategoryCount - 1)) {
			th_printf(",");
		}
	}
	th_printf("]\r\n");

	tf_freetensors();
}
