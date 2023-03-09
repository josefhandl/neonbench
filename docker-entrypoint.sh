#!/bin/bash

set -e

cd /neonbench

git clone https://github.com/josefhandl/neonbench.git || true

cd neonbench
git pull

cmake .
make

chown -R 1000:1000 ./neonbench
