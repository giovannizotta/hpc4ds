 #!/bin/bash

BIN="hpc4ds/project/bin/main.out"

N_NODES=4
N_CPU=4
RAM=32
DEBUG=1
rm -rf sub_scripts
mkdir -p sub_scripts


for MIN_SUPPORT in 0.05 0.1 0.2
do
    for N_PROC in 1 2 4
    do
        for N_THREAD in 1 2 4
        do
            for D in data/*
            do
                for FILE in ${D}*
                do
                    if [ -f "${FILE}" ]
                    then
                        FILENAME=$(basename ${FILE})
                        TIMESTAMP=$(date +"%Y-%m-%d_%H-%M-%S")
                        echo ${FILENAME}
                        echo ${TIMESTAMP}
                        sleep 1
cat > sub_scripts/${TIMESTAMP}_${MIN_SUPPORT}_${N_PROC}_${N_THREAD}_${FILENAME}_sub.sh <<EOF
#PBS -l select=${N_PROC}:ncpus=${N_THREAD}:mem=${RAM}gb:net_type=IB

#PBS -l walltime=0:50:30

#PBS -q short_cpuQ

#PBS -o out_${TIMESTAMP}_${MIN_SUPPORT}_${N_PROC}_${N_THREAD}_${FILENAME}

#PBS -e err_${TIMESTAMP}_${MIN_SUPPORT}_${N_PROC}_${N_THREAD}_${FILENAME}


module load mpich-3.2

/usr/bin/time -v mpirun.actual -n ${N_PROC} ${BIN} ${FILE} ${N_THREAD} ${MIN_SUPPORT} ${DEBUG} 
EOF
                        chmod +x sub_scripts/${TIMESTAMP}_${MIN_SUPPORT}_${N_PROC}_${N_THREAD}_${FILENAME}_sub.sh
                    fi
                done
            done
        done
    done
done

