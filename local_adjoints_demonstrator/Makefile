CXX ?= g++

FLAGS = -fopenmp -std=c++11 $(CXXFLAGS)

.PHONY: all
all: tests benchmark

headers: benchmark.hpp evaluation_strategies.hpp local_adjoints.hpp preaccumulations.hpp tape.hpp

tests: headers
	$(CXX) tests.cpp -o tests $(FLAGS) -O0 -ggdb
	
benchmark: headers
	$(CXX) benchmark.cpp -o benchmark $(FLAGS) -O3
	
.PHONY: clean
clean:
	rm -f tests
	rm -f benchmark
