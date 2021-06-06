 #!/bin/bash

BIN="hpc4ds/project/bin/main.out"

N_NODES=4
N_CPU=4
RAM=32
FILENAME="data/freq-items-tsz/0.5_tsz100_tct5.0m.txt" 
N_PROC=4
N_THREAD=4
MIN_SUPPORT=50
DEBUG=1

cat > test.txt <<EOF
#PBS -l select=${N_NODES}:ncpus=${N_CPU}:mem=${RAM}gb:net_type=IB

#PBS -l walltime=0:50:30

#PBS -q short_cpuQ

network $HPC
EOF