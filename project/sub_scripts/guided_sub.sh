#PBS -l select=16:ncpus=8:mem=64gb

#PBS -l walltime=0:50:30

#PBS -q short_cpuQ

#PBS -o /home/giovanni.zotta/hpc4ds/project/sub_results/pct/out_2021-06-18-00-57-16_30_0.0001_16_8_0.8_guided

#PBS -e /home/giovanni.zotta/hpc4ds/project/sub_results/pct/err_2021-06-18-00-57-16_30_0.0001_16_8_0.8_guided


module load mpich-3.2
export OMP_SCHEDULE=guided,1

/usr/bin/time -v mpirun.actual -n 16 hpc4ds/project/bin/main.out /home/giovanni.zotta/dataset/pct/0.8 8 0.0001 1 
