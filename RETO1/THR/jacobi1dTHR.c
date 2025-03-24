#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "timing.h"

#define MAX_THREAD 8

typedef struct {
    int start;
    int end;
    int n;
    double h2;
    double* u;
    double* utmp;
    double* f;
} thread_data_t;

/* Thread function for Jacobi iteration */
void* jacobi_thread(void* arg) {
    thread_data_t* data = (thread_data_t*) arg;
    for (int i = data->start; i < data->end; ++i) {
        data->utmp[i] = (data->u[i-1] + data->u[i+1] + data->h2 * data->f[i]) / 2;
    }
    return NULL;
}

/* --
 * Do nsweeps sweeps of Jacobi iteration on a 1D Poisson problem
 * 
 *    -u'' = f
 *
 * discretized by n+1 equally spaced mesh points on [0,1].
 * u is subject to Dirichlet boundary conditions specified in
 * the u[0] and u[n] entries of the initial vector.
 */
void jacobi(int nsweeps, int n, double* u, double* f)
{
    int sweep;
    double h  = 1.0 / n;
    double h2 = h*h;
    double* utmp = (double*) malloc( (n+1) * sizeof(double) );
    pthread_t threads[MAX_THREAD];
    thread_data_t thread_data[MAX_THREAD];
    int chunk_size = (n + MAX_THREAD - 1) / MAX_THREAD; // Ensure all elements are covered

    /* Fill boundary conditions into utmp */
    utmp[0] = u[0];
    utmp[n] = u[n];

    /* Initialize thread data */
    for (int t = 0; t < MAX_THREAD; ++t) {
        thread_data[t].start = t * chunk_size + 1;
        thread_data[t].end = (t + 1) * chunk_size + 1;
        if (thread_data[t].end > n) thread_data[t].end = n;
        thread_data[t].n = n;
        thread_data[t].h2 = h2;
        thread_data[t].u = u;
        thread_data[t].utmp = utmp;
        thread_data[t].f = f;
    }

    for (sweep = 0; sweep < nsweeps; sweep += 2) {
        /* Old data in u; new data in utmp */
        for (int t = 0; t < MAX_THREAD; ++t) {
            pthread_create(&threads[t], NULL, jacobi_thread, &thread_data[t]);
        }
        for (int t = 0; t < MAX_THREAD; ++t) {
            pthread_join(threads[t], NULL);
        }

        /* Old data in utmp; new data in u */
        for (int t = 0; t < MAX_THREAD; ++t) {
            thread_data[t].u = utmp;
            thread_data[t].utmp = u;
            pthread_create(&threads[t], NULL, jacobi_thread, &thread_data[t]);
        }
        for (int t = 0; t < MAX_THREAD; ++t) {
            pthread_join(threads[t], NULL);
        }
    }

    free(utmp);
}

void write_solution(int n, double* u, const char* fname)
{
    int i;
    double h = 1.0 / n;
    FILE* fp = fopen(fname, "w+");
    for (i = 0; i <= n; ++i)
        fprintf(fp, "%g %g\n", i*h, u[i]);
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

    /* Run the solver */    
    printf("n: %d\n"
           "nsteps: %d\n"
           "Elapsed time: %g s\n", 
           n, nsteps, (double)timespec_diff(tstart, tend));

    /* Write the results */
    if (fname)
        write_solution(n, u, fname);

    free(f);
    free(u);
    return 0;
}
