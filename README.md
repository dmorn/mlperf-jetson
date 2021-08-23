# mlperf-jetson
mlcommons tiny performance benchmark on **NVIDIA Jetson Nano**.

## Notes
The board's SD card needs to be populated with the proper software stack.
Pre-crafted images are provided at
https://developer.nvidia.com/embedded/downloads, but that method did not work
for me (the board freezes at boot splash-screen and I do not know how to debug
it). The sdkmanager flow worked. Install it following instructions provided at
https://developer.nvidia.com/nvidia-sdk-manager. Once installed it can flash a
connected board through usb/serial communication. I used this command:
```
% sdkmanager --cli install  --logintype devzone --product Jetson --targetos Linux --version 4.6 --target JETSON_NANO_TARGETS --flash all
```
Follow instructions provided by the tool. If you're running ubuntu 20.04 or
more, the current version of sdkmanager (1.6.0.8170, Thu 12 Aug 2021 12:17:37
PM CEST) will comply that it works only on ubuntu 18.xx.  Change
/etc/os-release VERSION_ID field to "18.04" and use it happily. Remember to
revert the change afterwards.

~~The serial line used is the one accessible through pins 8,10 of J41. In my
case, the associated device is `/dev/ttyTHS1`.~~
I'm using the microusb port now. It is more reliable, you just have to remember
stopping the getty service on it, see below. The device is `/dev/ttyGS0`.

The dtb is tegra210-p3448-0000-p3449-0000-a02.dtb. Extract the source issuing something like
```
% dtc -I dtb -O dts -o src.dts tegra210-p3448-0000-p3449-0000-a02.dtb
```

I had to disable getty on that serial line and I still need to determine which service
was referencing to that specific device (it must be an alias). Anyway
```
% systemctl stop serial-getty@ttyS0.service
% systemctl stop serial-getty@ttyGS0.service
% systemctl stop nvgetty.service 
```
suffices if you have to stop a specific getty service.

## About the model
```
% saved_model_cli show --dir kws_ref_model --tag_set serve --signature_def serving_default
The given SavedModel SignatureDef contains the following input(s):
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
```

## Other notes
```
2021-08-18 11:05:36.049422: W tensorflow/core/platform/profile_utils/cpu_utils.cc:108] Failed to find bogomips or clock in /proc/cpuinfo; cannot determine CPU frequency
```

## Links
- https://www.tensorflow.org/install/lang_c
- https://github.com/mlcommons/tiny/tree/master/v0.5/training/keyword_spotting
- https://qengineering.eu/install-tensorflow-2.3.1-on-jetson-nano.html
- https://www.tensorflow.org/guide/saved_model#load_a_savedmodel_in_c
- https://medium.com/@vladislavsd/undocumented-tensorflow-c-api-b527c0b4ef6
- https://docs.nvidia.com/deeplearning/frameworks/install-tf-jetson-platform/index.html
- https://blog.tensorflow.org/2021/01/leveraging-tensorflow-tensorrt-integration.html
- https://docs.nvidia.com/deeplearning/tensorrt/quick-start-guide/index.html
- https://developer.nvidia.com/blog/tensorrt-integration-speeds-tensorflow-inference/
- https://blog.tensorflow.org/2018/04/speed-up-tensorflow-inference-on-gpus-tensorRT.html
- https://docs.google.com/spreadsheets/d/e/2PACX-1vRhSPeEz1n6njCaWoATCDs2cbeG4hBGVkTzOhuXM4WCWaIogR9ialmai0VJ23WXWh_7VlI9w7O0wq2L/pubhtml?gid=0&single=false&widget=false&headers=false&chrome=true#
