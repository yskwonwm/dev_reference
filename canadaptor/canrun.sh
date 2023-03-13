#!/bin/sh

echo  "run can device"

sudo ip link set can0 up type can bitrate 500000
