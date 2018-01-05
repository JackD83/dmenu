#!/bin/sh
/bin/rm stdout.txt
/bin/rm stderr.txt
/bin/mount -o remount,rw /dev/mmcblk0p2 /usr/mtdblock3
/bin/mount -o remount, rw / >> stdout.txt 2>> stderr.txt
