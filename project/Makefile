FILENAME?="data/small_dataset.txt"
NTHREAD?=4

main: help

build:
	@mpicc -Wall -g -fopenmp src/*.c src/hashmap/*.c -o bin/main.out

run_local:
	@mpiexec -n $(NTHREAD) \
	bin/main.out $(FILENAME)

time_run_local:
	@/usr/bin/time -v mpiexec -n $(NTHREAD) \
	bin/main.out $(FILENAME)

all: clean build run_local 

clean: 
	@rm -f bin/*

help: 
	@echo "Not like this"


# valgrind --leak-check=yes \
# --show-reachable=yes --log-file=nc.vg.%p --suppressions=/usr/share/openmpi/openmpi-valgrind.supp \