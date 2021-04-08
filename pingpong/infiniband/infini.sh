#!/bin/bash
#PBS -l select=1:ncpus=2:mem=2gb:net_type=IB

#PBS -l walltime=0:10:30

#PBS -q short_cpuQ
module load mpich-3.2
mpirun.actual -n 2 pingpong/infiniband/infini
