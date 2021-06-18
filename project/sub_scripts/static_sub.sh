#PBS -l select=1:ncpus=8:mem=64gb

#PBS -l walltime=2:50:30

#PBS -q short_cpuQ

#PBS -o /home/giovanni.zotta/hpc4ds/project/sub_results/pct/out_2021-06-18-20-05-13_28_0.0001_1_8_0.6_static

#PBS -e /home/giovanni.zotta/hpc4ds/project/sub_results/pct/err_2021-06-18-20-05-13_28_0.0001_1_8_0.6_static


module load mpich-3.2
export OMP_SCHEDULE=static,1

/usr/bin/time -v mpirun.actual -n 1 hpc4ds/project/bin/main.out /home/giovanni.zotta/dataset2/pct/0.6 8 0.0001 1 
