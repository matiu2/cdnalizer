#!/bin/sh
cd /cdnalizer
git pull
cd build
cmake ..
make -j4
make package
