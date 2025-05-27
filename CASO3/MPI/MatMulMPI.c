// C para multiplicar dos matrices con MPI
#include <stdio.h>
#include <stdlib.h>
#include <mpi.h> // Include MPI header

// Helper function to allocate a matrix
int** allocate_matrix(int rows, int cols) {
    int** mat = (int**)malloc(rows * sizeof(int*));
    if (mat == NULL) {
        perror("Failed to allocate memory for matrix rows");
        MPI_Abort(MPI_COMM_WORLD, 1);
    }
    for (int i = 0; i < rows; i++) {
        mat[i] = (int*)malloc(cols * sizeof(int));
        if (mat[i] == NULL) {
            perror("Failed to allocate memory for matrix columns");
            // Free previously allocated rows before aborting
            for (int k = 0; k < i; k++) free(mat[k]);
            free(mat);
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
    }
    return mat;
}

// Helper function to free a matrix
void free_matrix(int** mat, int rows) {
    if (mat == NULL) return;
    for (int i = 0; i < rows; i++) {
        free(mat[i]);
    }
    free(mat);
}

// Computes a chunk of the result matrix.
// matA_chunk: num_rows_in_A_chunk x n_dim
// matB_full: n_dim x n_dim
// result_chunk: num_rows_in_A_chunk x n_dim
void compute_matrix_product_chunk(int** matA_chunk, int** matB_full, int** result_chunk, int num_rows_in_A_chunk, int n_dim) {
    for (int i = 0; i < num_rows_in_A_chunk; i++) {
        for (int j = 0; j < n_dim; j++) {
            result_chunk[i][j] = 0; // Initialize the result matrix element
            for (int k = 0; k < n_dim; k++) {
                result_chunk[i][j] += matA_chunk[i][k] * matB_full[k][j];
            }
        }
    }
}

int** generate_matrix(int size) {
    int** mat = allocate_matrix(size, size); // Use helper for allocation
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            mat[i][j] = rand() % 1001;;
        }
    }
    return mat;
}

void write_result(double time, int data) {
    FILE *f = fopen("resultados.txt", "a"); // Open file in append mode
    if (f == NULL) {
        printf("Error opening file!\n");
        exit(1);
    }
    fprintf(f, "Tiempo consumido: %.3f, para cantidad: %d\n", time, data);
    fclose(f); // Close the file
}

void print_result(int** result, int size) {
    FILE *f = fopen("matriz.txt", "w");
    if (f == NULL) {
        printf("Error opening file!\n");
        exit(1);
    }
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            fprintf(f, "%d ", result[i][j]);
        }
        fprintf(f, "\n");
    }
    fclose(f);
}

int main(int argc, char *argv[]) {
    int n;
    int doPrint = 0;

    MPI_Init(&argc, &argv);
    int world_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    double start_time, end_time, mpi_time_used;

    if (world_rank == 0) {
        if (argc != 2 && argc != 3) {
            fprintf(stderr, "Usage: %s <matrix_size> [print_flag]\n", argv[0]);
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
        n = atoi(argv[1]);
        if (argc == 3) {
            doPrint = atoi(argv[2]);
        }
        if (n <= 0) {
            fprintf(stderr, "Matrix size must be a positive integer.\n");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
        printf("The integer is: %d, number of MPI processes: %d\n", n, world_size);
    }

    // Broadcast matrix size n to all processes
    MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);

    int** matrixA = NULL;
    int** matrixB = NULL;
    int** result = NULL;

    int rows_for_master = (n / world_size) + (n % world_size);
    int rows_for_worker = n / world_size;

    if (world_rank == 0) {
        matrixA = generate_matrix(n);
        matrixB = generate_matrix(n);
        result = allocate_matrix(n, n);

        start_time = MPI_Wtime();

        // Send n, rows_for_worker, and matrixB to all workers.
        // Then send chunks of matrixA.
        int current_row_offset_A = rows_for_master;
        for (int dest = 1; dest < world_size; dest++) {
            MPI_Send(&rows_for_worker, 1, MPI_INT, dest, 0, MPI_COMM_WORLD);
            for (int i = 0; i < n; i++) { // Send matrixB row by row
                MPI_Send(matrixB[i], n, MPI_INT, dest, 1, MPI_COMM_WORLD);
            }
            for (int i = 0; i < rows_for_worker; i++) { // Send relevant rows of matrixA
                MPI_Send(matrixA[current_row_offset_A + i], n, MPI_INT, dest, 2, MPI_COMM_WORLD);
            }
            current_row_offset_A += rows_for_worker;
        }

        // Master computes its part
        // The first `rows_for_master` rows of matrixA are used, results stored in first `rows_for_master` rows of `result`
        compute_matrix_product_chunk(matrixA, matrixB, result, rows_for_master, n);

        // Receive results from workers
        int current_row_offset_result = rows_for_master;
        for (int source = 1; source < world_size; source++) {
            for (int i = 0; i < rows_for_worker; i++) {
                MPI_Recv(result[current_row_offset_result + i], n, MPI_INT, source, 3, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            }
            current_row_offset_result += rows_for_worker;
        }

        end_time = MPI_Wtime();
        mpi_time_used = (end_time - start_time) * 1000; // milliseconds

        write_result(mpi_time_used, n);
        printf("MPI multiplication took %.3f milliseconds to execute \n", mpi_time_used);
        if (doPrint != 0) {
            print_result(result, n);
        }

        free_matrix(matrixA, n);
        free_matrix(matrixB, n);
        free_matrix(result, n);

    } else { // Worker processes
        int my_rows;
        MPI_Recv(&my_rows, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        int** local_A_chunk = allocate_matrix(my_rows, n);
        int** matrixB_received = allocate_matrix(n, n);
        int** partial_result_chunk = allocate_matrix(my_rows, n);

        for (int i = 0; i < n; i++) { // Receive matrixB
            MPI_Recv(matrixB_received[i], n, MPI_INT, 0, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }
        for (int i = 0; i < my_rows; i++) { // Receive my rows of A
            MPI_Recv(local_A_chunk[i], n, MPI_INT, 0, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }

        compute_matrix_product_chunk(local_A_chunk, matrixB_received, partial_result_chunk, my_rows, n);

        for (int i = 0; i < my_rows; i++) { // Send partial result back to master
            MPI_Send(partial_result_chunk[i], n, MPI_INT, 0, 3, MPI_COMM_WORLD);
        }

        free_matrix(local_A_chunk, my_rows);
        free_matrix(matrixB_received, n);
        free_matrix(partial_result_chunk, my_rows);
    }

    MPI_Finalize();
    return 0;
}