/* tf.c provides a benchmark implementation using tensorflow */

#include <stddef.h>
#include <stdio.h>
#include <tensorflow/c/c_api.h>

#include "api/internally_implemented.h"
#include "api/submitter_implemented.h"

#define kNumCols 10
#define kNumRows 49
#define kNumChannels 1
#define kKwsInputSize kNumCols * kNumRows * kNumChannels
#define kCategoryCount 12

extern char *model_dir;
const char *tags = "serve";

char* kCategoryLabels[kCategoryCount] = {
	"down", "go", "left", "no", "off", "on",
	"right", "stop", "up", "yes", "silence", "unknown"
};

static TF_Status *s;
static TF_Session *sess;
static TF_Graph* g;
static TF_SessionOptions *opts;

/* Sizes are model and hardcoded here; they could be computed from model
 * shape itself, or shapes could be provided by a lookup table.
 */
static TF_Output ndi[1], ndo[1];
static TF_Tensor *tfi[1], *tfo[1];

void fatale(char *s);

void
tf_load(uint8_t *buf, size_t n) {
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

char*
tf_version(void) {
	return TF_Version();
}
