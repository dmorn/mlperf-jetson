#!/bin/sh

export TF_CPP_MIN_LOG_LEVEL=2

sudo systemctl stop serial-getty@ttyS0.service
sudo systemctl stop serial-getty@ttyGS0.service
sudo systemctl stop nvgetty.service 

sudo chmod 666 /dev/ttyTHS1
sudo chmod 666 /dev/ttyS0
sudo chmod 666 /dev/ttyS1
sudo chmod 666 /dev/ttyS2
sudo chmod 666 /dev/ttyGS0
