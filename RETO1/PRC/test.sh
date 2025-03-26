#!/bin/bash
#SBATCH --partition=all  	#Seleccione los nodos para el trabajo de todos el conjunto de nodos de cómputo del cluster
#SBATCH -o TEST/Serial.%j.out    	#Nombre del archivo de salida
#SBATCH -J Serial	        #Nombre del trabajo

source /usr/local/intel/parallel_studio_xe_2016.3.067/psxevars.sh intel64 2> /dev/null
ulimit -s unlimited -c unlimited

# Create the TEST directory if it doesn't exist
mkdir -p TEST

# Test1
N=100000
NSTEPS=1000
for i in {1..1}; do
    ./jacobi1d $N $NSTEPS TEST/timing_serial1.out > TEST/u_serial1.out
done

# Test2
N=100000
NSTEPS=10000
for i in {1..1}; do
    ./jacobi1d $N $NSTEPS TEST/timing_serial2.out > TEST/u_serial2.out
done

# Test3
N=1000000
NSTEPS=1000
for i in {1..1}; do
    ./jacobi1d $N $NSTEPS TEST/timing_serial3.out > TEST/u_serial3.out
done

# Test4
N=1000000
NSTEPS=10000
for i in {1..1}; do
    ./jacobi1d $N $NSTEPS TEST/timing_serial4.out > TEST/u_serial4.out
done

N=10000000
NSTEPS=10000
for i in {1..1}; do
    ./jacobi1d $N $NSTEPS TEST/timing_serial5.out > TEST/u_serial5.out
done