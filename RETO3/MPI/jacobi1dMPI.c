#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>

#include "timing.h"

/* --
 * MPI version of Jacobi iteration on a 1D Poisson problem
 */
void jacobi_mpi(int nsweeps, int n, double* u_local, double* f_local, 
                int local_n, int rank, int size, MPI_Comm comm)
{
    int i, sweep;
    double h  = 1.0 / n;
    double h2 = h*h;
    double* utmp = (double*) malloc( local_n * sizeof(double) );
    double left_neighbor = 0.0, right_neighbor = 0.0;
    
    // Copy boundary conditions for local array
    memcpy(utmp, u_local, local_n * sizeof(double));

    for (sweep = 0; sweep < nsweeps; sweep += 2) {
        
        // Exchange boundary values with neighbors
        if (rank > 0) {
            MPI_Sendrecv(&u_local[1], 1, MPI_DOUBLE, rank-1, 0,
                        &left_neighbor, 1, MPI_DOUBLE, rank-1, 0,
                        comm, MPI_STATUS_IGNORE);
        }
        if (rank < size-1) {
            MPI_Sendrecv(&u_local[local_n-2], 1, MPI_DOUBLE, rank+1, 0,
                        &right_neighbor, 1, MPI_DOUBLE, rank+1, 0,
                        comm, MPI_STATUS_IGNORE);
        }
        
        /* Old data in u_local; new data in utmp */
        for (i = 1; i < local_n-1; ++i) {
            double left_val = (i == 1 && rank > 0) ? left_neighbor : u_local[i-1];
            double right_val = (i == local_n-2 && rank < size-1) ? right_neighbor : u_local[i+1];
            utmp[i] = (left_val + right_val + h2*f_local[i])/2;
        }
        
        // Exchange boundary values for utmp
        if (rank > 0) {
            MPI_Sendrecv(&utmp[1], 1, MPI_DOUBLE, rank-1, 0,
                        &left_neighbor, 1, MPI_DOUBLE, rank-1, 0,
                        comm, MPI_STATUS_IGNORE);
        }
        if (rank < size-1) {
            MPI_Sendrecv(&utmp[local_n-2], 1, MPI_DOUBLE, rank+1, 0,
                        &right_neighbor, 1, MPI_DOUBLE, rank+1, 0,
                        comm, MPI_STATUS_IGNORE);
        }
        
        /* Old data in utmp; new data in u_local */
        for (i = 1; i < local_n-1; ++i) {
            double left_val = (i == 1 && rank > 0) ? left_neighbor : utmp[i-1];
            double right_val = (i == local_n-2 && rank < size-1) ? right_neighbor : utmp[i+1];
            u_local[i] = (left_val + right_val + h2*f_local[i])/2;
        }
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
        printf("n: %d\n"
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
    double* u_local;
    double* f_local;
    double h;
    timing_t tstart, tend;
    char* fname;
    
    int rank, size;
    int local_n, start_idx;
    
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    /* Process arguments */
    n      = (argc > 1) ? atoi(argv[1]) : 100;
    nsteps = (argc > 2) ? atoi(argv[2]) : 100;
    fname  = (argc > 3) ? argv[3] : NULL;
    h      = 1.0/n;

    // Calculate local domain size for each process
    int base_size = (n-1) / size;  // Interior points divided by processes
    int remainder = (n-1) % size;
    
    if (rank < remainder) {
        local_n = base_size + 1 + 2;  // +2 for boundary points
        start_idx = rank * (base_size + 1);
    } else {
        local_n = base_size + 2;      // +2 for boundary points
        start_idx = remainder * (base_size + 1) + (rank - remainder) * base_size;
    }

    /* Allocate local arrays */
    u_local = (double*) malloc( local_n * sizeof(double) );
    f_local = (double*) malloc( local_n * sizeof(double) );
    memset(u_local, 0, local_n * sizeof(double));

    // Initialize f_local
    for (i = 0; i < local_n; ++i) {
        int global_idx = start_idx + i;
        f_local[i] = global_idx * h;
    }
    
    // Set boundary conditions
    if (rank == 0) {
        u_local[0] = 0.0;  // Left boundary
    }
    if (rank == size-1) {
        u_local[local_n-1] = 0.0;  // Right boundary
    }

    /* Run the solver */
    MPI_Barrier(MPI_COMM_WORLD);
    if (rank == 0) get_time(&tstart);
    
    jacobi_mpi(nsteps, n, u_local, f_local, local_n, rank, size, MPI_COMM_WORLD);
    
    MPI_Barrier(MPI_COMM_WORLD);
    if (rank == 0) get_time(&tend);

    // Gather results to rank 0
    if (rank == 0) {
        u = (double*) malloc( (n+1) * sizeof(double) );
        f = (double*) malloc( (n+1) * sizeof(double) );
        
        // Copy local data from rank 0
        memcpy(u, u_local, local_n * sizeof(double));
        int current_pos = local_n;
        
        // Receive data from other processes
        for (int src = 1; src < size; src++) {
            int src_local_n;
            if (src < remainder) {
                src_local_n = base_size + 1 + 2;
            } else {
                src_local_n = base_size + 2;
            }
            
            double* temp_u = (double*) malloc(src_local_n * sizeof(double));
            MPI_Recv(temp_u, src_local_n, MPI_DOUBLE, src, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            
            // Copy interior points (skip overlapping boundary)
            memcpy(&u[current_pos-1], &temp_u[1], (src_local_n-1) * sizeof(double));
            current_pos += src_local_n - 1;
            
            free(temp_u);
        }
        
        // Initialize f for output
        for (i = 0; i <= n; ++i)
            f[i] = i * h;
    } else {
        // Send local data to rank 0
        MPI_Send(u_local, local_n, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
    }

    if (rank == 0) {
        if (fname){
            write_results(n, nsteps, tstart, tend, fname);
        }
        
        free(f);
        free(u);
    }

    free(f_local);
    free(u_local);
    
    MPI_Finalize();
    return 0;
}
