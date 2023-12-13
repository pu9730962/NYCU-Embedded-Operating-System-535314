#!/bin/sh

set -x
# set -e

rmmod -f mydev
insmod mydev.ko

./writer Devin &
./reader 192.168.20.200 8888 /dev/mydev
