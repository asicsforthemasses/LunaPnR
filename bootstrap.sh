#!/bin/sh

mkdir -p build
cd build
cmake -GNinja -DCMAKE_BUILD_TYPE=Debug ..
