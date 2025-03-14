// C para multiplicar dos matrices

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
// maximum number of threads
#define MAX_THREAD 4

struct thread_data {
    int** matA;
    int** matB;
    int** result;
    int size;
    int i;
    int j;
};
void* multi(void* arg) {
    struct thread_data* data = (struct thread_data*)arg;
    int** matA = data->matA;
    int** matB = data->matB;
    int** result = data->result;
    int size = data->size;
    int thread_id = data->i;

    for (int i = thread_id * size / MAX_THREAD; i < (thread_id + 1) * size / MAX_THREAD; i++) {
        for (int j = 0; j < size; j++) {
            result[i][j] = 0;
            for (int k = 0; k < size; k++) {
                result[i][j] += matA[i][k] * matB[k][j];
            }
        }
    }
    pthread_exit(0);
}

int** multiply_matrices_threads(int** matA, int** matB, int** result, int size) {
    pthread_t threads[MAX_THREAD];
    struct thread_data thread_data_array[MAX_THREAD];

    for (int i = 0; i < MAX_THREAD; i++) {
        thread_data_array[i].matA = matA;
        thread_data_array[i].matB = matB;
        thread_data_array[i].result = result;
        thread_data_array[i].size = size;
        thread_data_array[i].i = i;
        pthread_create(&threads[i], NULL, multi, (void*)&thread_data_array[i]);
    }

    for (int i = 0; i < MAX_THREAD; i++) {
        pthread_join(threads[i], NULL);
    }

    return result;
}

int** generate_matrix(int size) {
    int** mat = (int**)malloc(size * sizeof(int*));
    for (int i = 0; i < size; i++) {
        mat[i] = (int*)malloc(size * sizeof(int));
    }
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            mat[i][j] = rand() % 1001;
        }
    }
    return mat;
}

void write_time_taken(double time, int data) {
    FILE *f = fopen("resultados.txt", "a"); // Open file in append mode
    if (f == NULL) {
        printf("Error opening file!\n");
        exit(1);
    }
    fprintf(f, "Tiempo consumido: %f, para cantidad: %d\n", time, data);
    fclose(f); // Close the file
}

void print_result(int** result, int size){
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            printf("%d ", result[i][j]);
        }
        printf("\n");
    }
}

void initialize_matrices(int*** matrixA, int*** matrixB, int*** result, int size) {
    *matrixA = generate_matrix(size);
    *matrixB = generate_matrix(size);
    *result = (int**)malloc(size * sizeof(int*));
    for (int i = 0; i < size; i++) {
        (*result)[i] = (int*)malloc(size * sizeof(int));
    }
}

void free_matrices(int** matrixA, int** matrixB, int** result, int size) {
    for (int i = 0; i < size; i++) {
        free(matrixA[i]);
        free(matrixB[i]);
        free(result[i]);
    }
    free(matrixA);
    free(matrixB);
    free(result);
}

int main(int argc, char *argv[]) {
    int doPrint = 0;
    if (argc != 2 ) {
        if (argc != 3) {
            fprintf(stderr, "Usage: %s <integer>\n", argv[0]);
            return 1;
        } else {
            doPrint = atoi(argv[2]);
        }
    }
    int n = atoi(argv[1]);
    printf("The integer is: %d\n", n);

    int** matrixA;
    int** matrixB;
    int** result;
    clock_t start, end;
    double cpu_time_used;

    initialize_matrices(&matrixA, &matrixB, &result, n);

    start = clock();
    result = multiply_matrices_threads(matrixA, matrixB, result, n);
    end = clock();

    cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
    write_time_taken(cpu_time_used, n);
    printf("multiplication took %f seconds to execute \n", cpu_time_used);

    if (doPrint != 0) {
        print_result(result, n);
    }

    free_matrices(matrixA, matrixB, result, n);

    return 0;
}