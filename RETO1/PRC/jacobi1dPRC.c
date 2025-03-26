#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>

#include "timing.h"

#define MAX_PROCESS 16

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
    int i, sweep, pid[MAX_PROCESS], status;
    double h  = 1.0 / n;
    double h2 = h * h;
    double* utmp = (double*) malloc((n + 1) * sizeof(double));
    int chunk_size = n / MAX_PROCESS;
    int pipes[MAX_PROCESS][2]; // Pipes for synchronization

    /* Fill boundary conditions into utmp */
    utmp[0] = u[0];
    utmp[n] = u[n];

    // Create pipes for communication
    for (int p = 0; p < MAX_PROCESS; ++p) {
        if (pipe(pipes[p]) == -1) {
            perror("pipe");
            exit(EXIT_FAILURE);
        }
    }

    for (int p = 0; p < MAX_PROCESS; ++p) {
        pid[p] = fork();
        if (pid[p] == 0) { // Child process
            close(pipes[p][0]); // Close read end of the pipe
            for (sweep = 0; sweep < nsweeps; sweep++) {
                int start = p * chunk_size + 1;
                int end = (p == MAX_PROCESS - 1) ? n : (p + 1) * chunk_size;

                if (sweep % 2 == 0) {
                    /* Old data in u; new data in utmp */
                    for (i = start; i < end; ++i)
                        utmp[i] = (u[i - 1] + u[i + 1] + h2 * f[i]) / 2;
                } else {
                    /* Old data in utmp; new data in u */
                    for (i = start; i < end; ++i)
                        u[i] = (utmp[i - 1] + utmp[i + 1] + h2 * f[i]) / 2;
                }

                // Notify parent that this process is done
                if (write(pipes[p][1], "done", 4) != 4) {
                    perror("write");
                    exit(EXIT_FAILURE);
                }
            }
            close(pipes[p][1]); // Close write end of the pipe
            exit(0); // Child process exits
        }
    }

    for (sweep = 0; sweep < nsweeps; sweep++) {
        // Wait for all child processes to complete their work
        for (int p = 0; p < MAX_PROCESS; ++p) {
            char buffer[4];
            close(pipes[p][1]); // Close write end of the pipe in parent
            if (read(pipes[p][0], buffer, 4) != 4) { // Wait for "done" from child
                perror("read");
                exit(EXIT_FAILURE);
            }
        }
    }

    // Wait for all child processes to finish
    for (int p = 0; p < MAX_PROCESS; ++p) {
        waitpid(pid[p], &status, 0);
        close(pipes[p][0]); // Close read end of the pipe
    }

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
//