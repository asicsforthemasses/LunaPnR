#!/bin/sh

rm -rf build
mkdir -p build
cd build
cmake -GNinja -DCMAKE_BUILD_TYPE=Debug -DUseCPPCHECK=ON -DUseCLANGTIDY=ON ..
