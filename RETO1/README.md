# Reto 1: Jacobi poisson 1d
Se hicieron pruebas en secuencial, y con paralelismo con hilos/procesos.  
Fueron 6 series de pruebas con los siguientes datos:
- N=100000, NSTEPS=1000
- N=100000, NSTEPS=10000
- N=1000000, NSTEPS=1000
- N=1000000, NSTEPS=10000
- N=10000000, NSTEPS=10000
- N=10000000, NSTEPS=100000

## Compilacion
### Secuential version
icc -DUSE_CLOCK -O3 jacobi1d.c timing.c -o jacobi1d
gcc -DCLOCK_REALTIME -O3 jacobi1d.c timing.c -o jacobi1d -lm
### Threaded version
gcc -DCLOCK_REALTIME -O3 jacobi1dTHR.c timing.c -o jacobi1d -pthread -lm
### Proccess version
gcc -DCLOCK_REALTIME -O3 jacobi1dPROC.c timing.c -o jacobi1d -pthread -lm

## Ejecucion:
chmod +x serial.sh
./serial.sh

## Perfilamiento
gcc -pg -O3 jacobi1d.c timing.c -o jacobi1d -lm
./jacobi1d 1000000 10000 output.txt
