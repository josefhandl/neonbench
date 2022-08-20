#!/bin/bash


clang -Wall -Wextra -Werror -g -msse -fPIC -shared -o sse.so sse.c

clang++ -Wall -Wextra -Werror -ldl -mavx -o benchmark benchmark.cpp

