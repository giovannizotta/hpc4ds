#!/bin/bash

#PBS -l select=4:ncpus=4:mem=64gb:net_type=IB

#PBS -l walltime=0:20:30

#PBS -q short_cpuQ

BIN="hpc4ds/project/bin/main.out"
N_NODES=4
N_CPU=4
RAM=32

FILENAME="data/freq-items-tsz/0.5_tsz100_tct5.0m.txt" 
N_PROC=4
N_THREAD=16
MIN_SUPPORT=50
DEBUG=1

module load mpich-3.2
echo "nodes=${N_NODES} ncpus=${N_CPU} ram=${RAM}"
echo "file=${FILENAME} nproc=${N_PROC} nthreads=${N_THREAD} minsupport=${MIN_SUPPORT}"

/usr/bin/time -v mpirun.actual -n ${N_PROC} ${BIN} ${FILENAME} ${N_THREAD} ${MIN_SUPPORT} ${DEBUG} 

# #PBS -o stdout_file

# #PBS -e stderr_file
