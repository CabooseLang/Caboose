#!/bin/bash

set -ex

mkdir -p build
cd build

cmake .. -B. -H..
cmake --build .

make package