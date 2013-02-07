#!/bin/bash
# 

DEPDIR=./external_deps

mkdir $DEPDIR 
cd $DEPDIR

wget -i ../external-files


tar zxf libcsv-3.0.1.tar.gz
cd libcsv-3.0.1
make -j8
sudo make install
cd ..
#
tar zxf papi-4.2.0.tar.gz
cd papi-4.2.0/src
./configure && make -j8
sudo make install
cd ../..
# 
tar zxf metis-5.0.2.tar.gz
cd metis-5.0.2
make config
make -j8
sudo make install

cd ..

