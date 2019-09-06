#!/bin/sh

set -ex

cd build
cmake .. -B. -H..
cmake --build .

make package