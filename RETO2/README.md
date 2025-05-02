# Reto 2: Jacobi poisson 1d
Se hicieron pruebas en secuencial, y con paralelismo con hilos/procesos.  
Fueron 6 series de pruebas con los siguientes datos:
- N=100000, NSTEPS=1000
- N=100000, NSTEPS=10000
- N=1000000, NSTEPS=1000
- N=1000000, NSTEPS=10000
- N=10000000, NSTEPS=10000
- N=10000000, NSTEPS=100000

## Compilacion
### Version secuencial
gcc -DCLOCK_REALTIME jacobi1d.c timing.c -o jacobi1d -lm
gcc -DCLOCK_REALTIME -O3 jacobi1d.c timing.c -o jacobi1do3 -lm
### Version OpenMP 8 hilos
gcc -DCLOCK_REALTIME -fopenmp jacobi1d8Thr.c timing.c -o jacobi1d8T -lm
gcc -DCLOCK_REALTIME -O3 -fopenmp jacobi1d8Thr.c timing.c -o jacobi1d8To3 -lm
### Version OpenMP 16 hilos
gcc -DCLOCK_REALTIME -fopenmp jacobi1d16Thr.c timing.c -o jacobi1d16T -lm
gcc -DCLOCK_REALTIME -O3 -fopenmp jacobi1d16Thr.c timing.c -o jacobi1d16To3 -lm

## Ejecucion:
### Version secuencial
chmod +x secuential.sh
./secuential.sh
### Version OpenMP 8 hilos
chmod +x openmp8.sh
./openmp8.sh
### Version OpenMP 16 hilos
chmod +x openmp16.sh
./openmp16.sh
