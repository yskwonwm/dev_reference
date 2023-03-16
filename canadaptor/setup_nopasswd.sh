#!/usr/bin/expect

spawn bash -c "sudo ip link set can0 up type can bitrate 500000";
expect -re "password"
send "86001\r"
interact


