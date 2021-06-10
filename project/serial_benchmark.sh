 #!/bin/bash

BIN="hpc4ds/project/bin/main.out"
DATA_DIR=$1
MAIN_FILE=$2

N_NODES=1
N_CPU=1
RAM=128
DEBUG=1
rm -rf sub_scripts
mkdir -p sub_scripts
mkdir -p sub_results

for D in ${DATA_DIR}/*
do
    mkdir -p sub_results/$(basename ${D})
    # echo $(basename ${D})
done

submit () {
    D=$1
    FILE=$2
    ITER=$3
    MIN_SUPPORT=$4
    N_PROC=$5
    N_THREAD=$6
    if [ -f "${FILE}" ]
    then
        FILENAME=$(basename ${FILE})
        TIMESTAMP=$(date +"%Y-%m-%d-%H-%M-%S")
        SUB_NAME=${TIMESTAMP}_${ITER}_${MIN_SUPPORT}_${N_PROC}_${N_THREAD}_${FILENAME}
        echo ${SUB_NAME}
cat > sub_scripts/sub.sh <<EOF
#PBS -l select=${N_NODES}:ncpus=${N_CPU}:mem=${RAM}gb

#PBS -l walltime=2:50:30

#PBS -q short_cpuQ

#PBS -o /home/giovanni.zotta/hpc4ds/project/sub_results/$(basename ${D})/out_${SUB_NAME}

#PBS -e /home/giovanni.zotta/hpc4ds/project/sub_results/$(basename ${D})/err_${SUB_NAME}


module load mpich-3.2

/usr/bin/time -v mpirun.actual -n ${N_PROC} ${BIN} ${FILE} ${N_THREAD} ${MIN_SUPPORT} ${DEBUG} 
EOF
        chmod +x sub_scripts/sub.sh
        qsub sub_scripts/sub.sh
        sleep 1000
    fi
}

cycle_files_dir () {
    MIN_SUPPORT=$1
    for ITER in 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15
    do
        for D in ${DATA_DIR}/*
        do
            for FILE in ${D}/*
            do
                submit "${D}" "${FILE}" "${ITER}" "${MIN_SUPPORT}" "1" "1"
            done
        done
    done
}

cycle_main_file () {
    MIN_SUPPORT=$1
    for ITER in 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15
    do
        submit "${D}" "${MAIN_FILE}" "${ITER}" "${MIN_SUPPORT}" "1" "1"
    done
}

cycle_files "0"