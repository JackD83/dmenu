#! /bin/sh

#If can't bulid menu without errors, then stop
if ! scripts/build.sh; then
    echo "Error in build process, please check before building"
    exit
fi

BASE=`pwd -P`
REL_ROOT=release
REL_FULL=$REL_ROOT/local/dmenu

#Clean up host files, only leave dingoo
make -f Makefile.host clean

#Prep build area
rm -rf $REL_FULL
mkdir -p $REL_FULL

#copy over basic file
for x in changelog.txt readme.txt; do
  cp $BASE/$x $REL_ROOT
done

#Copy over global assets, executables
for x in resources wallpapers dmenu*; do
    cp -r $BASE/$x $REL_FULL
done

#Add default theme
mkdir -p $REL_FULL/themes/default
cp -r $BASE/themes/default/* $REL_FULL/themes/default

rm -f $BASE/release-$1.zip
cd $REL_ROOT
zip -r -x"*.svn*" -x"*.thumb*" -x"*-host*" $BASE/release-$1.zip .
cd $BASE
rm -rf $REL_ROOT

