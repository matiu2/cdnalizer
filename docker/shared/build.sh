#!/bin/sh
cd /cdnalizer
git pull
cd build
cmake ..
make
ctest
make package
aws s3 cp $PACKAGE s3://cdnalizer-packages/${DEST}/
