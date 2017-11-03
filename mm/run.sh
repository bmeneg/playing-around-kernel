#!/bin/bash

MOD_NAME="my-alloc.ko"

sudo rmmod $MOD_NAME
make
sudo insmod $MOD_NAME
dmesg | tail
