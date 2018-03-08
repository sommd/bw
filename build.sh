#!/usr/bin/env sh

set -e

autoreconf --install
./configure
make
ln -s src/bw bw
