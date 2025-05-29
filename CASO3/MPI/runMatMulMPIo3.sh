#!/bin/bash
# Script: runMatMulMPI.sh
# Asume que mycluster.txt está en ../ (directorio CASO3)

HOSTFILE="../mycluster.txt"
EXECUTABLE="./matMulMPIo3" # Para la versión optimizada

for size in 500 1000 1500 2000 3500 5000
do
    echo "Running MPIo3: $EXECUTABLE with size $size"
    for i in {1..10} # repeticiones
    do
        mpiexec -n 4 --hostfile $HOSTFILE $EXECUTABLE $size
    done
done