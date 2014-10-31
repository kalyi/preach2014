all:
	clang++ -std=c++11 -O3 -o preach++ main.cc ConstructionGraph.cc QueryGraph.cc Fifo.cc
