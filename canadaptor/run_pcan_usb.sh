#!/bin/sh
modprobe peak_usb
ip link set can0 up type can bitrate 500000
