

ALLFILES=$(wildcard src/*.h) $(wildcard src/avx2/*.h) $(wildcard src/generic/*.h) $(wildcard src/sse/*.h)

unit: tests/unit.cpp $(ALLFILES)
	c++ -std=c++17 -o unit tests/unit.cpp -I src


clean:
	rm -r -f unit