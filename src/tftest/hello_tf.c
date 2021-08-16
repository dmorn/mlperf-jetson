#include <stdio.h>
#include <tensorflow/c/c_api.h>

int main() {
	char *ver;
	int code;
	TF_Graph* g;
	TF_DeviceList *devs;
	TF_Status *s;
	TF_Session *sess;
	TF_SessionOptions *opts;

	ver = TF_Version();
	printf("Hello from TensorFlow C library version %s\n", ver);

	g = TF_NewGraph();
	opts = TF_NewSessionOptions();
	sess = TF_NewSession(g, opts, s);
	if((code = TF_GetCode(s)) != TF_OK) {
		perror(TF_Message(s));
		return code;
	}
	devs = TF_SessionListDevices(sess, s);
	if((code = TF_GetCode(s)) != TF_OK) {
		perror(TF_Message(s));
		return code;
	}
	printf("Found %d devices\n", TF_DeviceListCount(devs));
	for(int i = 0; i < TF_DeviceListCount(devs); i++) {
		char *name;

		name = TF_DeviceListName(devs, i, s);
		if((code = TF_GetCode(s)) != TF_OK) {
			perror(TF_Message(s));
			return code;
		}
		printf("%d: %s\n", i, name);
	}

	TF_DeleteDeviceList(devs);
	TF_CloseSession(sess, s);
	TF_DeleteGraph(g);
	TF_DeleteSessionOptions(opts);

	return 0;
}
