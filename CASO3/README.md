# Caso 3: multiplicacion de matrices en cluster
Se hicieron pruebas en un solo núcleo de un solo nodo.
Y pruebas en múltiples núcleos de un solo nodo(con hilos OpenMP).
Fueron pruebas con las siguientes dimensiones: 500, 1000, 1500, 2000, 3500 y 5000.

## Compilacion
### Secuential version
gcc -O3 -o matMulo3 MatMul.c
gcc -o matMul MatMul.c
### OpenMP version
gcc -O3 -fopenmp MatMulOpenMP.c -o matMul8o3 -lm
gcc -fopenmp MatMulOpenMP.c -o matMul8 -lm
### MPI version
mpicc -o matMulMPI MatMulMPI.c -O3
mpicc -o matMulMPI MatMulMPI.c
## Ejecucion:
chmod +x runMatMul.sh
dos2unix ./runMatMul.sh
./runMatMul.sh
### MPI
mpiexec -n 4 --hostfile ~/CASO3/mycluster.txt /home/cluser/CASO3/MPI/matMulMPI 500