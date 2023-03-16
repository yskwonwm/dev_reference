#!/bin/sh

echo  "run can device"

sudo modprobe peak_usb
sudo ip link set can0 up type can bitrate 500000
