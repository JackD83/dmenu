#!/bin/bash
HOST_ERRS=0
DINGOO_ERRS=0

rm -rf wallpapers/.thumb
rm -f *~ */*~ */*/*~

make -f Makefile.host rebuild 0> /dev/null 1> /dev/null 2> scripts/host_err
HOST_ERRS=`cat scripts/host_err | grep "\(warning\|error\)" | wc -l`

if [ -z $1 ]; then
    make -f Makefile.dingoo rebuild 0> /dev/null 1> /dev/null 2> scripts/dingoo_err
    DINGOO_ERRS=`cat scripts/dingoo_err | grep "\(warning\|error\)" | wc -l`
fi

if [ $HOST_ERRS -gt 0 ]; then
  echo 
  echo "Host Errors"
  echo "-----------------------------------------"
  cat scripts/host_err | grep -v make
  echo 
fi

if [ $DINGOO_ERRS -gt 0 ]; then
  echo 
  echo "Dingoo Errors"
  echo "-----------------------------------------"
  cat scripts/dingoo_err | grep -v make
  echo 
fi

rm -f scripts/host_err scripts/dingoo_err

if [ $((HOST_ERRS + DINGOO_ERRS)) -gt 0 ]; then
   exit 1 
fi
