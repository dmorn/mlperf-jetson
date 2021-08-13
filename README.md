# mlperf-jetson
mlcommons tiny performance benchmark on NVIDIA Jetson Nano.

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


## Next
- p3448-0000.conf
- https://www.tensorflow.org/install/lang_c
- https://github.com/mlcommons/tiny/tree/master/v0.5/training/keyword_spotting
