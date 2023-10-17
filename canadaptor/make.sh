#!/bin/sh

echo  "making canadaptor"

g++ -o canadaptor main.cpp data_relayer.cpp can_adaptor.cpp can_send.cpp can_dump.cpp lib.cpp -lpthread 
