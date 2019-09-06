#!/bin/sh

set -ex
cmake . -Bbuild -H.
cmake --build ./build

cd build
make package