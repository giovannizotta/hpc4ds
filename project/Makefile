FILENAME?="data/small_dataset.txt"
NPROC?=4
NTHREAD?=1

main: help

build:
	@mpicc -std=gnu99 -Wall -g -fopenmp src/*.c src/hashmap/*.c -o bin/main.out

run_local:
	@mpiexec -n $(NPROC) \
	bin/main.out $(FILENAME) $(NTHREAD)

time_run_local:
	@/usr/bin/time -v mpiexec -n $(NPROC) \
	bin/main.out $(FILENAME) $(NTHREAD)

all: clean build run_local 

clean: 
	@rm -f bin/*

help: 
	@echo "Not like this"


# valgrind --leak-check=yes \
# --show-reachable=yes --log-file=nc.vg.%p --suppressions=/usr/share/openmpi/openmpi-valgrind.supp \
