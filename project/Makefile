FILENAME?="data/small_dataset.txt"
NTHREAD?=4

build:
	@mpicc -Wall src/*.c -o bin/main

run_local:
	@mpiexec bin/main $(FILENAME) -n $(NTHREAD)

all: clean build run_local

clean: 
	@rm bin/*
