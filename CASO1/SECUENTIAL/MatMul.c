// C para multiplicar dos matrices
#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int** multiply_matrices_save(int** matA, int** matB, int** result, int size) {
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

int** multiply_matrices(int** matA, int** matB, int size) {
    int temp_res;
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            temp_res = 0; // Initialize the result matrix element
            for (int k = 0; k < size; k++) {
                temp_res += matA[i][k] * matB[k][j];
            }
            //printf("%d ", result[i][j]);
        }
        //printf("\n");
    }
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
    result = multiply_matrices_save(matrixA, matrixB, result, n); //guardar resultados
    clock_gettime(CLOCK_MONOTONIC, &end);
    //cpu_time_used = (end.tv_sec - start.tv_sec) + ((double)(end.tv_nsec - start.tv_nsec)) / 1e9; // Segundos
    ms_time = (end.tv_sec - start.tv_sec) * 1000 + ((double)(end.tv_nsec - start.tv_nsec)) / 1e6; // Miliseg
    // Compilar con: gcc -pg -o matMul matMul.c
    // Ejecutar con: ./matMul <argumentos>
    // Analizar con: gprof matMul gmon.out > analysis.txt

    write_result(ms_time,n);
    printf("multiplication took %.3f milliseconds to execute \n", ms_time); //mostrar el tiempo en ms
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