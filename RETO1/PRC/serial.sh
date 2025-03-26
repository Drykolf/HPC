#!/bin/bash
#SBATCH --partition=all  	#Seleccione los nodos para el trabajo de todos el conjunto de nodos de cómputo del cluster
#SBATCH -o OUT/Serial.%j.out    	#Nombre del archivo de salida
#SBATCH -J Serial	        #Nombre del trabajo

source /usr/local/intel/parallel_studio_xe_2016.3.067/psxevars.sh intel64 2> /dev/null
ulimit -s unlimited -c unlimited

# Create the OUT directory if it doesn't exist
mkdir -p OUT

# Test1
N=100000
NSTEPS=1000
for i in {1..1}; do
    ./jacobi1d $N $NSTEPS OUT/timing_serial1.out > OUT/u_serial1.out
done

# Test2
N=100000
NSTEPS=10000
for i in {1..1}; do
    ./jacobi1d $N $NSTEPS OUT/timing_serial2.out > OUT/u_serial2.out
done

# Test3
N=1000000
NSTEPS=1000
for i in {1..1}; do
    ./jacobi1d $N $NSTEPS OUT/timing_serial3.out > OUT/u_serial3.out
done

# Test4
N=1000000
NSTEPS=10000
for i in {1..1}; do
    ./jacobi1d $N $NSTEPS OUT/timing_serial4.out > OUT/u_serial4.out
done

# Test5
N=10000000
NSTEPS=10000
for i in {1..1}; do
    ./jacobi1d $N $NSTEPS OUT/timing_serial5.out > OUT/u_serial5.out
done

# Test6
N=10000000
NSTEPS=100000
for i in {1..1}; do
    ./jacobi1d $N $NSTEPS OUT/timing_serial6.out > OUT/u_serial6.out
done