#!/bin/bash

#g++ -msse -c -o sse.o sse.c
g++ -msse -fPIC -shared sse.c -o sse.so

g++ -ldl -mavx -o benchmark benchmark.cpp

