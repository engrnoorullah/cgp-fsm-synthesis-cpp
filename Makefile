CXX ?= c++
CXXFLAGS ?= -std=c++17 -O2 -Wall -Wextra -Wpedantic -Werror

.PHONY: all test clean
all: build/cgp

build:
	mkdir -p build

build/cgp: src/main.cpp src/cgp.cpp src/cgp.hpp | build
	$(CXX) $(CXXFLAGS) src/main.cpp src/cgp.cpp -o $@

build/test_cgp: tests/test_cgp.cpp src/cgp.cpp src/cgp.hpp | build
	$(CXX) $(CXXFLAGS) tests/test_cgp.cpp src/cgp.cpp -o $@

test: all build/test_cgp
	./build/test_cgp
	./build/cgp train examples/xor.table build/xor.cgp --seed 10 --generations 10000
	./build/cgp check build/xor.cgp examples/xor.table
	./build/cgp train examples/sequence_101.table build/sequence_101.cgp --seed 10 --generations 100000
	./build/cgp check build/sequence_101.cgp examples/sequence_101.table
	python3 tests/test_cli.py

clean:
	rm -rf build
