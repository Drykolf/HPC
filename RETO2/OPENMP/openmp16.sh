#!/bin/bash
#SBATCH --partition=all  	#Seleccione los nodos para el trabajo de todos el conjunto de nodos de cómputo del cluster
#SBATCH -o OUT/16T/Serial.%j.out    	#Nombre del archivo de salida
#SBATCH -J Serial	        #Nombre del trabajo

source /usr/local/intel/parallel_studio_xe_2016.3.067/psxevars.sh intel64 2> /dev/null
ulimit -s unlimited -c unlimited

# Create the OUT directory if it doesn't exist
mkdir -p OUT
mkdir -p OUT/16T


# Test1
N=100000
NSTEPS=1000
for i in {1..6}; do
    ./jacobi1d16T $N $NSTEPS OUT/16T/timing_16T_openmp1.out
done

# Test2
N=100000
NSTEPS=10000
for i in {1..6}; do
    ./jacobi1d16T $N $NSTEPS OUT/16T/timing_16T_openmp2.out
done

# Test3
N=1000000
NSTEPS=1000
for i in {1..6}; do
    ./jacobi1d16T $N $NSTEPS OUT/16T/timing_16T_openmp3.out
done

# Test4
N=1000000
NSTEPS=10000
for i in {1..6}; do
    ./jacobi1d16T $N $NSTEPS OUT/16T/timing_16T_openmp4.out
done

# Test5
N=10000000
NSTEPS=10000
for i in {1..6}; do
    ./jacobi1d16T $N $NSTEPS OUT/16T/timing_16T_openmp5.out
done

# Test6
N=10000000
NSTEPS=100000
for i in {1..6}; do
    ./jacobi1d16T $N $NSTEPS OUT/16T/timing_16T_openmp6.out
done

#optimization -O3
# Test1
N=100000
NSTEPS=1000
for i in {1..6}; do
    ./jacobi1d16To3 $N $NSTEPS OUT/16T/timing_16T_o3openmp1.out
done

# Test2
N=100000
NSTEPS=10000
for i in {1..6}; do
    ./jacobi1d16To3 $N $NSTEPS OUT/16T/timing_16T_o3openmp2.out
done

# Test3
N=1000000
NSTEPS=1000
for i in {1..6}; do
    ./jacobi1d16To3 $N $NSTEPS OUT/16T/timing_16T_o3openmp3.out
done

# Test4
N=1000000
NSTEPS=10000
for i in {1..6}; do
    ./jacobi1d16To3 $N $NSTEPS OUT/16T/timing_16T_o3openmp4.out
done

# Test5
N=10000000
NSTEPS=10000
for i in {1..6}; do
    ./jacobi1d16To3 $N $NSTEPS OUT/16T/timing_16T_o3openmp5.out
done

# Test6
N=10000000
NSTEPS=100000
for i in {1..6}; do
    ./jacobi1d16To3 $N $NSTEPS OUT/16T/timing_16T_o3openmp6.out
done