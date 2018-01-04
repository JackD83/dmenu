#!/bin/bash
if [ -z "$1" ]; then
   echo "You must specify the mount point for the script to run.  ex: deploy.sh /media/dingoo"
   exit 1
fi

MNT=$( echo $1 | sed -e 's/\/$//g')
DMN="$MNT/local/dmenu"
FILES="dmenu dmenu.bin dmenu.ini"
FOLDERS="resources wallpapers"
NEED_MOUNT=1

if mount | grep "on $MNT type" > /dev/null; then 
    NEED_MOUNT=0
fi

if [ $NEED_MOUNT -gt 0 ]; then 
   if ! mount $MNT; then
     exit 1 
   fi
fi

if ./scripts/build.sh; then
   rm -rf $DMN/wallpapers/.thumb
   for x in $FILES; do 
       cp $x $DMN
   done
   for y in $FOLDERS; do
       for x in $(ls $y/*); do
           cp $x $DMN/$y
       done
   done
fi

if [ $NEED_MOUNT -gt 0 ]; then 
    sleep 3
    umount $MNT
fi
