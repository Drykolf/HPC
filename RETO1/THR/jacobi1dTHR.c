#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "timing.h"

#define MAX_THREADS 16

typedef struct {
    int start;
    int end;
    int n;
    double h2;
    double* u;
    double* f;
    double* utmp;
} thread_data_t;

pthread_barrier_t barrier;

void* jacobi_thread(void* arg) {
    thread_data_t* data = (thread_data_t*)arg;
    double left = data->u[data->start - 1];
    for (int i = data->start; i < data->end; ++i) {
        double right = data->u[i + 1];
        data->utmp[i] = (left + right + data->h2 * data->f[i]) / 2;
        left = data->u[i];
    }
    pthread_barrier_wait(&barrier);
    return NULL;
}

void jacobi(int nsweeps, int n, double* u, double* f) {
    int sweep;
    double h  = 1.0 / n;
    double h2 = h * h;
    double* utmp = (double*) malloc((n+1) * sizeof(double));
    pthread_t threads[MAX_THREADS];
    thread_data_t thread_data[MAX_THREADS];
    int chunk_size = (n + MAX_THREADS - 1) / MAX_THREADS; // Ceiling division

    pthread_barrier_init(&barrier, NULL, MAX_THREADS);

    /* Fill boundary conditions into utmp */
    utmp[0] = u[0];
    utmp[n] = u[n];

    for (sweep = 0; sweep < nsweeps; sweep += 2) {
        /* Old data in u; new data in utmp */
        for (int t = 0; t < MAX_THREADS; ++t) {
            thread_data[t].start = t * chunk_size + 1;
            thread_data[t].end = (t == MAX_THREADS - 1) ? n : (t + 1) * chunk_size;
            thread_data[t].n = n;
            thread_data[t].h2 = h2;
            thread_data[t].u = u;
            thread_data[t].f = f;
            thread_data[t].utmp = utmp;
            pthread_create(&threads[t], NULL, jacobi_thread, &thread_data[t]);
        }
        for (int t = 0; t < MAX_THREADS; ++t) {
            pthread_join(threads[t], NULL);
        }

        /* Old data in utmp; new data in u */
        for (int t = 0; t < MAX_THREADS; ++t) {
            thread_data[t].u = utmp;
            thread_data[t].utmp = u;
            pthread_create(&threads[t], NULL, jacobi_thread, &thread_data[t]);
        }
        for (int t = 0; t < MAX_THREADS; ++t) {
            pthread_join(threads[t], NULL);
        }
    }

    pthread_barrier_destroy(&barrier);
    free(utmp);
}

void write_solution(int n, double* u, const char* fname)
{
    int i;
    double h = 1.0 / n;
    //FILE* fp = fopen(fname, "w+");
    for (i = 0; i <= n; ++i)
        printf("%g %g\n", i*h, u[i]);
    //fclose(fp);
}

void write_results(int n, int nsteps, timing_t tstart, timing_t tend, const char* fname)
{
    FILE* fp = fopen(fname, "a");
        if (fp == NULL) {
            perror("Error opening file");
            return;
        }
        fprintf(fp, "n: %d\n"
                    "nsteps: %d\n"
                    "Elapsed time: %g s\n", 
                    n, nsteps, (double)timespec_diff(tstart, tend));
        fclose(fp);
}
int main(int argc, char** argv)
{
    int i;
    int n, nsteps;
    double* u;
    double* f;
    double h;
    timing_t tstart, tend;
    char* fname;

    /* Process arguments */
    n      = (argc > 1) ? atoi(argv[1]) : 100;
    nsteps = (argc > 2) ? atoi(argv[2]) : 100;
    fname  = (argc > 3) ? argv[3] : NULL;
    h      = 1.0/n;

    /* Allocate and initialize arrays */
    u = (double*) malloc( (n+1) * sizeof(double) );
    f = (double*) malloc( (n+1) * sizeof(double) );
    memset(u, 0, (n+1) * sizeof(double));
    for (i = 0; i <= n; ++i)
        f[i] = i * h;

    /* Run the solver */
    get_time(&tstart);
    jacobi(nsteps, n, u, f);
    get_time(&tend);

    /* Run the solver     
    printf("n: %d\n"
           "nsteps: %d\n"
           "Elapsed time: %g s\n", 
           n, nsteps, timespec_diff(tstart, tend));
    */
    if (fname){
        write_results(n, nsteps, tstart, tend, fname);
    }
    /* Write the results */
    //if (fname)
        //write_solution(n, u, fname);

    free(f);
    free(u);
    return 0;
}

//Se porciona la carga en chunks de tamaño n/threads, se crea un arreglo de threads y se les asigna un rango de trabajo, se crea un barrier para sincronizar los threads y se ejecuta el jacobi_thread en cada thread, se espera a que todos los threads terminen
//Se agregó una barrier para sincronizar los threads y reducir la cantidad de threads creados
//Se compila usando -DCLOCK_REALTIME para medir el tiempo tomado