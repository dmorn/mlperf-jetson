#!/bin/sh

export TF_CPP_MIN_LOG_LEVEL=2

sudo systemctl stop serial-getty@ttyS0.service
sudo systemctl stop serial-getty@ttyGS0.service
sudo systemctl stop nvgetty.service 
sudo chmod 666 /dev/ttyTHS1
