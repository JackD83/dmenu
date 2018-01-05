#!/bin/sh

    echo "" > /sys/devices/platform/musb_hdrc.0/gadget/gadget-lun0/file
    echo "" > /sys/devices/platform/musb_hdrc.0/gadget/gadget-lun1/file
    
    /bin/mount -t vfat -o rw,utf8 /dev/mmcblk0p4 /mnt/memory

		MMC_STATUS=`cat /proc/jz/mmc`
		
		if [ -e /dev/mmcblk1p1 ]
		then
		      MMC_NAME=mmcblk1p1
		      
		elif [ -e /dev/mmcblk1 ]
		then
					MMC_NAME=mmcblk1
		
		elif [ -e /dev/mmcblk2p1 ]
		then
		      MMC_NAME=mmcblk2p1	
		      	
		elif [ -e /dev/mmcblk2 ]
		then    
		      MMC_NAME=mmcblk2
		fi
	
					
		if [ x"$MMC_STATUS" = x"INSERT" ]
		then
			    #MMC_NAME=`cat /var/MMCNAME`
			    
					if [ x"$MMC_NAME" != x ] && [ x"$MMC_NAME" != x"no" ]
					then
								/bin/mount -t vfat -o rw,utf8,noatime /dev/$MMC_NAME /mnt/mmc
		            VALUE=$?
								sleep 0.5
								if [ $VALUE -ne 0 ]
								then
								    	/bin/mount -t exfat -o rw,utf8,noatime /dev/$MMC_NAME /mnt/mmc
								fi
					fi
		fi
		
		/bin/sync
