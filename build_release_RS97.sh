#!/bin/sh

make -f Makefile.dingoo RS97=1
cd volume 
make -f Makefile.dingux
cd ..
cd viewimage
make -f Makefile.dingux
cd ..

mkdir -p build_release/dmenu
cp -a installer build_release/dmenu
cp -a resources build_release/dmenu
cp -a scripts build_release/dmenu
cp -a themes_RS97 build_release/dmenu/themes
cp -a wallpapers build_release/dmenu

mkdir -p build_release/dmenu/usb
cp viewimage/usb_RS97.png build_release/dmenu/usb/usb.png
cp viewimage/viewimage.dge build_release/dmenu/usb

mkdir -p build_release/dmenu/volume
cp volume/setVolume build_release/dmenu/volume

cp dmenu_RS97 build_release/dmenu/dmenu
cp dmenuc.bin build_release/dmenu
cp sdlfix.so build_release/dmenu

cp dmenu.ini build_release/dmenu
cp SwitchMenu.dge build_release/dmenu