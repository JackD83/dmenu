#!/bin/sh

USB_STATE=`cat /sys/devices/platform/musb_hdrc.0/uh_cable`
if [ $USB_STATE == "usb" ]
then

		/bin/sync
		
		umount -f /mnt/memory
		echo /dev/mmcblk0p4 > /sys/devices/platform/musb_hdrc.0/gadget/gadget-lun0/file

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
								umount -f /mnt/mmc		
								echo /dev/$MMC_NAME > /sys/devices/platform/musb_hdrc.0/gadget/gadget-lun1/file
					fi
		fi
		
fi
/bin/sync
/tmp/viewimage.dge
/usr/bin/udc_disconnect.sh