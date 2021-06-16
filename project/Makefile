FILENAME?="data/small_dataset.txt"
N_PROC?=4
N_THREAD?=1
MIN_SUPPORT?=0.2
DEBUG?=0

main: help

all: clean build run_local 

build_test:
	@mpicc -O2 -std=gnu99 -Wall -g -fopenmp test_schedule.c -o bin/test_schedule.out

build:
	@mpicc -O2 -std=gnu99 -Wall -g -fopenmp src/*.c src/hashmap/*.c -o bin/main.out

run_local:
	@mpiexec -n $(N_PROC) \
	bin/main.out $(FILENAME) $(N_THREAD) $(MIN_SUPPORT) $(DEBUG)

time_run_local:
	@/usr/bin/time -v mpiexec -n $(N_PROC) \
	bin/main.out $(FILENAME) $(N_THREAD) $(MIN_SUPPORT) $(DEBUG)

clean: 
	@rm -f bin/*

help: 
	@echo "Not like this"

latex_doc:
	@doxygen config.conf
	@cd doc/doxygen/latex && make all

show_latex_doc:
	@evince doc/doxygen/latex/refman.pdf

clean_doc:
	@rm -rf doc/doxygen


# valgrind --leak-check=yes \
# --show-reachable=yes --log-file=nc.vg.%p --suppressions=/usr/share/openmpi/openmpi-valgrind.supp \
