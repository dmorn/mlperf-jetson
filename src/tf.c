/* tf.c provides a benchmark implementation using tensorflow */

#include <stddef.h>
#include <tensorflow/c/c_api.h>

#include "api/submitter_implemented.h"

extern char *model_dir;
const char *tags = "serve";

/* Sizes are model and hardcoded here; they could be computed from model
 * shape itself, or shapes could be provided by a lookup table.
 */
static TF_Output ndi[1], ndo[1];
static TF_Tensor *tfi[1], *tfo[1];
static TF_Session *sess;

void fatale(char *s);

void
tf_init(void) {
	TF_SessionOptions *opts;
	TF_Status *s;
	TF_Graph* g; // Leaked.
	uint8_t buf[kKwsInputSize];
	float *res;

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
	fprintf(stderr, "initializing tensorflow..");

	g = TF_NewGraph();
	s = TF_NewStatus();
	opts = TF_NewSessionOptions();
	sess = TF_LoadSessionFromSavedModel(opts, NULL, model_dir, &tags, 1, g, NULL, s);
	if(TF_GetCode(s) != TF_OK) {
		fatale(TF_Message(s));
	}
	TF_DeleteSessionOptions(opts);

	if((ndi->oper = TF_GraphOperationByName(g, "serving_default_input_1")) == NULL)
		fatale("input operation not found within graph");
	ndi->index = 0;

	if((ndo->oper = TF_GraphOperationByName(g, "StatefulPartitionedCall")) == NULL)
		fatale("output operation not found within graph");
	ndo->index = 0;

	/* Make a first inference to force tensorflow load all runtime dynamic
	 * libraries (\o/) */
	tf_load(buf, kKwsInputSize);
	tf_infer();
	res = tf_results();
	tf_freetensors();

	fprintf(stderr, ".done\n");
}

void
tf_load(uint8_t *buf, size_t n) {
	size_t len = n * sizeof(float);
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
tf_infer(void) {
	TF_Status *s;

	s = TF_NewStatus();
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
tf_freetensors(void) {
	TF_DeleteTensor(*tfi);
	TF_DeleteTensor(*tfo);
}

/* Free tensors after calling this function with tf_freetensors */
float*
tf_results(void) {
	if(*tfo == NULL)
		fatale("output tensor is empty");
	if(TF_TensorElementCount(*tfo) != kCategoryCount)
		fatale("unexpected tensor element count");

	return (float*)TF_TensorData(*tfo);
}

char*
tf_version(void) {
	return TF_Version();
}
