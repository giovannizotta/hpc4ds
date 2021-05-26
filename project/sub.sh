#!/bin/bash
#PBS -l select=4:ncpus=4:mem=16gb:net_type=IB

#PBS -l walltime=0:10:30

#PBS -q short_cpuQ
module load mpich-3.2
/usr/bin/time -v mpirun.actual -n 16 hpc4ds/project/bin/main.out data/freq-items-tsz/0.5_tsz10_tct5.0m.txt 4
