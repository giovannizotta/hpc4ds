FILENAME?="data/small_dataset.txt"
N_PROC?=4
N_THREAD?=1
MIN_SUPPORT?=1
DEBUG?=0

main: help

build:
	@mpicc -std=gnu99 -Wall -g -fopenmp src/*.c src/hashmap/*.c -o bin/main.out

run_local:
	@mpiexec -n $(N_PROC) \
	bin/main.out $(FILENAME) $(N_THREAD) $(MIN_SUPPORT) $(DEBUG)

time_run_local:
	@/usr/bin/time -v mpiexec -n $(N_PROC) \
	bin/main.out $(FILENAME) $(N_THREAD) $(MIN_SUPPORT) $(DEBUG)

all: clean build run_local 

clean: 
	@rm -f bin/*

help: 
	@echo "Not like this"


# valgrind --leak-check=yes \
# --show-reachable=yes --log-file=nc.vg.%p --suppressions=/usr/share/openmpi/openmpi-valgrind.supp \
