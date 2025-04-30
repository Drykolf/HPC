// C para multiplicar dos matrices
#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h> // Añadimos OpenMP

// Definimos el número de hilos a utilizar
#define NUM_THREADS 16

int** multiply_matrices(int** matA, int** matB, int** result, int size) {
    #pragma omp parallel for num_threads(NUM_THREADS)
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            result[i][j] = 0; // Initialize the result matrix element
            for (int k = 0; k < size; k++) {
                result[i][j] += matA[i][k] * matB[k][j];
            }
            //printf("%d ", result[i][j]);
        }
        //printf("\n");
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
    int doPrint = 0;
    if (argc != 2 ) {
        if (argc != 3) {
            fprintf(stderr, "Usage: %s <integer>\n", argv[0]);
            return 1;
        }
        else{doPrint = atoi(argv[2]);}
    }
    int n = atoi(argv[1]);
    printf("The integer is: %d\n", n);
    printf("Using %d threads with OpenMP\n", NUM_THREADS);
    int** matrixA;
    int** matrixB;
    //Generar matrices
    matrixA = generate_matrix(n);
    matrixB = generate_matrix(n);
    int** result = (int**)malloc(n * sizeof(int*));
    for (int i = 0; i < n; i++) {
        result[i] = (int*)malloc(n * sizeof(int));
    }
    struct timespec start, end;
    double cpu_time_used, ms_time;
    clock_gettime(CLOCK_MONOTONIC, &start);
    result = multiply_matrices(matrixA, matrixB, result, n); //guardar resultados
    clock_gettime(CLOCK_MONOTONIC, &end);
    //cpu_time_used = (end.tv_sec - start.tv_sec) + ((double)(end.tv_nsec - start.tv_nsec)) / 1e9; // Segundos
    ms_time = (end.tv_sec - start.tv_sec) * 1000 + ((double)(end.tv_nsec - start.tv_nsec)) / 1e6; // Miliseg
    // Compilar con: gcc -o matMul matMul.c -fopenmp
    // Optimizado: gcc -O3 -fopenmp MatMul.c -o matMul -lm
    // Ejecutar con: ./matMul <argumentos>

    write_result(ms_time,n);
    printf("multiplication took %.3f milliseconds to execute with %d threads\n", ms_time, NUM_THREADS); //mostrar el tiempo en ms
    if(doPrint != 0){
        print_result(result,n);
    }
    // Free allocated memory
    for (int i = 0; i < n; i++) {
        free(matrixA[i]);
        free(matrixB[i]);
        free(result[i]);
    }
    free(matrixA);
    free(matrixB);
    free(result);

    return 0;
}