## CAN adaptor
CAN communication transmit/receive connection

## 개발환경
* ubuntu 20.04
* gcc version 9.4.0   
* Visual Studio Code 1.75.1
* Can-utils

## Architecture  
![image](https://user-images.githubusercontent.com/87844157/220793454-9d475db8-2912-4f9f-b98d-b82f2806458d.png)

## Source discription

CAN adpator 관련 소스 및 테스트용  can utils 파일 
* can adaptor
    * can_adaptor.cpp
    * can_adaptor.hpp
    * can_dump.cpp
    * can_dump.hpp
    * can_send.cpp
    * can_send.hpp
    * data_relayer.hpp
    * main.cpp	
    * cancallbackfunc.hpp
* can utils
    * candump.c
    * cansend.c
    * lib.c
    * lib.h
* include
    * w1candbc.h     

## Complie
 ```
# can adaptor 
$ g++ -o canadaptor main.cpp can_adaptor.cpp can_dump.cpp lib.c -lpthread    


# can send
$ g++ -o cansend cansend.c lib.c

# can dump
$ g++ -o candump candump.c lib.c

```         

## CAN Setting ( for real can )
```
1) 장비 start시에 장치 추가
/etc/udev/rules.d 아래 rules파일에 다음 내용 추가
ACTION=="add|change",RUN+="/usr/local/bin/run_pcan_usb.sh”

2) 장치 추가 쉘스크립트
$vi /usr/local/bin/run_pcan_usb.sh

#!/bin/sh
modprobe peak_usb
ip link set can0 up type can bitrate 500000
ip link set can1 up type can bitrate 500000

```         

## Test ( for vcan )
 ```
# create vcan device 
$ sudo ip link add name vcan0 type vcan    
$ sudo ip link set up vcan0

# run can adapter   
$ ./canadaptor

# Run CAN data receiving program for test
$ ./candump vcan0,0x506:7FF,0x504:7FF,0x503:7FF,0x502:7FF,0x501:7FF
 
# Run CAN data transmission program for test   
$./cansend vcan1 169#0500010000000000
$./cansend vcan1 10b#0100000001000000
$./cansend vcan1 10a#0101010101010000
$./cansend vcan1 142#0100010001000000
$./cansend vcan1 154#0000020000010000
$./cansend vcan0 010#0500100010000101

``` 

## Reference 
* [linux can](https://elinux.org/CAN_Bus)
* [SocketCAN](https://github.com/linux-can)
* [BUSMASTER](https://rbei-etas.github.io/busmaster)
