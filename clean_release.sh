#!/bin/bash

make -f Makefile.dingoo clean
make -f Makefile.host clean

cd volume
make -f Makefile.dingux clean
cd ..
cd viewimage
make -f Makefile.dingux clean
cd ..

rm -rf build_release