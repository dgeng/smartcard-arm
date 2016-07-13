#!/bin/sh
arm-linux-gcc -I/home/proj/ajit/linux/include -Wall -Wstrict-prototypes -O2  -pipe -msoft-float -mshort-load-bytes -march=armv4 -mtune=strongarm110 -fno-strict-aliasing -c -o smartpoll.o smartpoll.c
