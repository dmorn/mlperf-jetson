#include <stdio.h>
#include <tensorflow/c/c_api.h>

const char *tags = "serve";

int
usage(char *name) {
	fprintf(stderr, "usage %s [tensorflow saved model dir path]\n", name);
	return 1;
}

int main(int argc, char *argv[]) {
	char *ver, *dir;
	int code;
	TF_Graph* g;
	TF_DeviceList *devs;
	TF_Status *s;
	TF_Session *sess;
	TF_SessionOptions *opts;

	if(argc != 2)
		usage(argv[0]);
	dir = argv[1];

	ver = TF_Version();
	printf("Hello from TensorFlow C library version %s\n", ver);

	g = TF_NewGraph();
	opts = TF_NewSessionOptions();

	sess = TF_LoadSessionFromSavedModel(opts, NULL, dir, &tags, 1, g, NULL, s);
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

	TF_DeleteGraph(g);

	TF_CloseSession(sess, s);
	TF_DeleteSession(sess, s);

	TF_DeleteDeviceList(devs);
	TF_DeleteSessionOptions(opts);

	return 0;
}
