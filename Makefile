
CC=clang
CCPP=clang++

CFLAGS=-g -Wall -Wextra -Werror -pedantic

LIBS=sse.so avx.so avx512f.so

.PHONY: all libs clean

all: benchmark libs

libs: $(LIBS)

%.so: %.c
	$(CC) $(CFLAGS) -fPIC -shared -m$(patsubst %.so,%,$@) -o $@ $^

benchmark: benchmark.cpp
	$(CCPP) $(CFLAGS) -mavx -ldl -o $@ $^

clean:
	rm -f benchmark $(LIBS)
