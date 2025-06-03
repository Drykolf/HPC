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
    
    // Copy initial values
    memcpy(utmp, u_local, local_n * sizeof(double));

    for (sweep = 0; sweep < nsweeps; sweep++) {
        
        // Non-blocking communication to avoid deadlocks
        MPI_Request req_send_left = MPI_REQUEST_NULL, req_recv_left = MPI_REQUEST_NULL;
        MPI_Request req_send_right = MPI_REQUEST_NULL, req_recv_right = MPI_REQUEST_NULL;
        
        // Start non-blocking communication
        if (rank > 0 && local_n > 2) {
            MPI_Isend(&u_local[1], 1, MPI_DOUBLE, rank-1, 100, comm, &req_send_left);
            MPI_Irecv(&left_neighbor, 1, MPI_DOUBLE, rank-1, 101, comm, &req_recv_left);
        }
        if (rank < size-1 && local_n > 2) {
            MPI_Isend(&u_local[local_n-2], 1, MPI_DOUBLE, rank+1, 101, comm, &req_send_right);
            MPI_Irecv(&right_neighbor, 1, MPI_DOUBLE, rank+1, 100, comm, &req_recv_right);
        }
        
        // Wait for communication to complete
        if (req_recv_left != MPI_REQUEST_NULL) MPI_Wait(&req_recv_left, MPI_STATUS_IGNORE);
        if (req_recv_right != MPI_REQUEST_NULL) MPI_Wait(&req_recv_right, MPI_STATUS_IGNORE);
        if (req_send_left != MPI_REQUEST_NULL) MPI_Wait(&req_send_left, MPI_STATUS_IGNORE);
        if (req_send_right != MPI_REQUEST_NULL) MPI_Wait(&req_send_right, MPI_STATUS_IGNORE);
        
        /* Update interior points */
        for (i = 1; i < local_n-1; ++i) {
            double left_val, right_val;
            
            if (i == 1 && rank > 0) {
                left_val = left_neighbor;
            } else {
                left_val = u_local[i-1];
            }
            
            if (i == local_n-2 && rank < size-1) {
                right_val = right_neighbor;
            } else {
                right_val = u_local[i+1];
            }
            
            utmp[i] = (left_val + right_val + h2*f_local[i])/2.0;
        }
        
        // Copy back to u_local instead of swapping pointers
        memcpy(u_local, utmp, local_n * sizeof(double));
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
    double* u = NULL;
    double* f = NULL;
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

    // Debug output for rank 0
    if (rank == 0) {
        printf("Starting Jacobi MPI with n=%d, nsteps=%d, processes=%d\n", n, nsteps, size);
        fflush(stdout);
    }

    // Simplified domain decomposition - ensure minimum work per process
    int interior_points = n - 1;  // Points between boundaries
    
    if (interior_points < size) {
        if (rank == 0) {
            fprintf(stderr, "Error: Too many processes (%d) for problem size (n=%d)\n", size, n);
            fprintf(stderr, "Maximum processes for this problem: %d\n", interior_points);
        }
        MPI_Finalize();
        return 1;
    }
    
    int base_size = interior_points / size;
    int remainder = interior_points % size;
    
    // Calculate local domain size including ghost points
    int local_interior;
    if (rank < remainder) {
        local_interior = base_size + 1;
        start_idx = rank * (base_size + 1) + 1;
    } else {
        local_interior = base_size;
        start_idx = remainder * (base_size + 1) + (rank - remainder) * base_size + 1;
    }
    
    // Add boundary/ghost points
    local_n = local_interior + 2;

    // Debug output
    if (rank == 0) {
        printf("Domain decomposition: base_size=%d, remainder=%d\n", base_size, remainder);
        printf("Rank %d: local_interior=%d, local_n=%d, start_idx=%d\n", 
               rank, local_interior, local_n, start_idx);
        fflush(stdout);
    }

    /* Allocate local arrays */
    u_local = (double*) calloc(local_n, sizeof(double));
    f_local = (double*) calloc(local_n, sizeof(double));
    
    if (!u_local || !f_local) {
        fprintf(stderr, "Memory allocation failed on rank %d\n", rank);
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    // Initialize f_local with correct global indices
    for (i = 0; i < local_n; ++i) {
        int global_idx = start_idx - 1 + i;
        if (global_idx < 0) global_idx = 0;
        if (global_idx > n) global_idx = n;
        f_local[i] = global_idx * h;
    }
    
    // Set boundary conditions for actual boundary processes
    if (rank == 0) {
        u_local[0] = 0.0;  // Left boundary
    }
    if (rank == size-1) {
        u_local[local_n-1] = 0.0;  // Right boundary
    }

    // Debug: Confirm initialization
    MPI_Barrier(MPI_COMM_WORLD);
    if (rank == 0) {
        printf("Initialization complete, starting Jacobi iterations...\n");
        fflush(stdout);
    }

    /* Run the solver */
    MPI_Barrier(MPI_COMM_WORLD);
    if (rank == 0) get_time(&tstart);
    
    jacobi_mpi(nsteps, n, u_local, f_local, local_n, rank, size, MPI_COMM_WORLD);
    
    MPI_Barrier(MPI_COMM_WORLD);
    if (rank == 0) {
        get_time(&tend);
        printf("Jacobi iterations complete, gathering results...\n");
        fflush(stdout);
    }

    // Gather results with error checking
    if (rank == 0) {
        u = (double*) malloc((n+1) * sizeof(double));
        f = (double*) malloc((n+1) * sizeof(double));
        
        if (!u || !f) {
            fprintf(stderr, "Memory allocation failed for result arrays\n");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
    }
    
    // Gather using simpler approach
    int* recvcounts = NULL;
    int* displs = NULL;
    
    if (rank == 0) {
        recvcounts = (int*) malloc(size * sizeof(int));
        displs = (int*) malloc(size * sizeof(int));
        
        // Calculate receive counts and displacements
        int current_disp = 1; // Start after left boundary
        for (i = 0; i < size; i++) {
            if (i < remainder) {
                recvcounts[i] = base_size + 1;
            } else {
                recvcounts[i] = base_size;
            }
            displs[i] = current_disp;
            current_disp += recvcounts[i];
        }
        
        // Set boundaries
        u[0] = 0.0;  // Left boundary
        u[n] = 0.0;  // Right boundary
    }
    
    // Each process sends its interior points (skip boundary/ghost points)
    double* send_data = &u_local[1];
    MPI_Gatherv(send_data, local_interior, MPI_DOUBLE,
                u ? &u[1] : NULL, recvcounts, displs, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        printf("Results gathered successfully\n");
        fflush(stdout);
        
        if (fname) {
            write_results(n, nsteps, tstart, tend, fname);
        }
        
        // Initialize f for completeness
        for (i = 0; i <= n; ++i)
            f[i] = i * h;
            
        free(recvcounts);
        free(displs);
        free(f);
        free(u);
    }

    free(f_local);
    free(u_local);
    
    MPI_Finalize();
    return 0;
}
