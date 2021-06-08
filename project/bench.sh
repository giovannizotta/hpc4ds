 #!/bin/bash

BIN="hpc4ds/project/bin/main.out"
DATA_DIR=$1

N_NODES=4
N_CPU=4
RAM=64
DEBUG=1
rm -rf sub_scripts
mkdir -p sub_scripts
rm -rf sub_results
mkdir -p sub_results

for D in ${DATA_DIR}/*
do
    mkdir -p sub_results/$(basename ${D})
    # echo $(basename ${D})
done

for ITER in 1 2 3 4 5 
do
    for MIN_SUPPORT in 0.0001 0.001 0.005
    do
        for N_PROC in 1 2 4
        do
            for N_THREAD in 1 2 4
            do
                for D in ${DATA_DIR}/*
                do
                    for FILE in ${D}/*
                    do
                        if [ -f "${FILE}" ]
                        then
                            FILENAME=$(basename ${FILE})
                            TIMESTAMP=$(date +"%Y-%m-%d-%H-%M-%S")
                            SUB_NAME=${TIMESTAMP}_${ITER}_${MIN_SUPPORT}_${N_PROC}_${N_THREAD}_${FILENAME}
                            echo ${SUB_NAME}
                            sleep 1
cat > sub_scripts/sub.sh <<EOF
#PBS -l select=${N_PROC}:ncpus=${N_CPU}:mem=${RAM}gb:net_type=IB

#PBS -l walltime=0:50:30

#PBS -q short_cpuQ

#PBS -o sub_results/$(basename ${D})/out_${SUB_NAME}

#PBS -e sub_results/$(basename ${D})/err_${SUB_NAME}


module load mpich-3.2

/usr/bin/time -v mpirun.actual -n ${N_PROC} ${BIN} ${FILE} ${N_THREAD} ${MIN_SUPPORT} ${DEBUG} 
EOF
                            chmod +x sub_scripts/sub.sh
                            qsub sub_scripts/sub.sh
                        fi
                    done
                done
            done
        done
    done
done

