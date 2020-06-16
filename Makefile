

ALLFILES=$(wildcard src/*.h) $(wildcard src/avx2/*.h) $(wildcard src/generic/*.h) $(wildcard src/sse/*.h) $(wildcard src/neon/*.h)
BENCHMARKFILES=$(wildcard benchmarks/*.h)  $(wildcard benchmarks/*.cpp)
all: unit benchmark

CPPFLAGS:=-O3  -std=c++17 -Wextra -Wall

unit: tests/unit.cpp $(ALLFILES)
	c++ $(CPPFLAGS) -o unit tests/unit.cpp -I src

random_utf8.o: benchmarks/random_utf8.h  benchmarks/random_utf8.cpp
	c++  $(CPPFLAGS) -c benchmarks/random_utf8.cpp -Ibenchmarks

benchmark: tests/unit.cpp $(ALLFILES) $(BENCHMARKFILES)  random_utf8.o
	c++  $(CPPFLAGS) -o benchmark benchmarks/benchmark.cpp -Ibenchmarks -Isrc random_utf8.o

clean:
	rm -r -f unit random_utf8.o benchmark