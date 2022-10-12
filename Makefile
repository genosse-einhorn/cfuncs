
CC     := gcc
CXX    := g++
CFLAGS := -Og -g -Wall -Wextra -Wformat=2 -Wconversion -Wshadow -Wpointer-arith

ALL := \
    test/c11/test-vector \
    test/c11/test-str \
    test/c11/test-str-list \
    test/c11/test-intrusive-list \
    test/c11/test-hashtbl \
    test/c11/test-hashtbl2 \
    test/c99/test-vector \
    test/c99/test-str \
    test/c99/test-str-list \
    test/c99/test-intrusive-list \
    test/c99/test-hashtbl \
    test/c99/test-hashtbl2 \
    test/c++/test-vector \
    test/c++/test-str \
    test/c++/test-str-list \
    test/c++/test-intrusive-list \
    test/c++/test-hashtbl \
    test/c++/test-hashtbl2 \
    test-str \
    test-str-list \
    test-intrusive-list \
    test-hashtbl \
    test-hashtbl2

all: $(ALL)

test/c99/%: %.c $(wildcard *.h) Makefile
	@mkdir -p test/c99
	$(CC) -std=c99 $(CFLAGS) -o $@ $<

test/c11/%: %.c $(wildcard *.h) Makefile
	@mkdir -p test/c11
	$(CC) -std=c11 $(CFLAGS) -o $@ $<

test/c++/%: %.c $(wildcard *.h) Makefile
	@mkdir -p test/c++
	$(CXX) -std=c++11 $(CFLAGS) -o $@ $<

%: %.c $(wildcard *.h) Makefile
	$(CC) -std=c11 $(CFLAGS) -o $@ $<

clean:
	rm -f $(ALL)

