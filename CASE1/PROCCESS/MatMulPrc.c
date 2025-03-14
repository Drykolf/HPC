// C para multiplicar dos matrices

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>
#include <unistd.h>
int test()
{

    // make two process which run same
    // program after this instruction
    pid_t p = fork();
    if(p<0){
      perror("fork fail");
      exit(1);
    }
    printf("Hello world!, process_id(pid) = %d \n",getpid());
    return 0;
}

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
            mat[i][j] = rand() % 1001;
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
    fprintf(f, "Tiempo consumido: %f, para cantidad: %d\n", time, data);
    fclose(f); // Close the file
}

void printResult(int** result, int size){
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            printf("%d ", result[i][j]);
        }
        printf("\n");
    }
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
    clock_t start, end;
    double cpu_time_used;
    start = clock();
    result = multiply_matrices_save(matrixA, matrixB, result, n); //guardar resultados
    end = clock();
    cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;

    // Compilar con: gcc -pg -o matMul matMul.c
    // Ejecutar con: ./matMul <argumentos>
    // Analizar con: gprof matMul gmon.out > analysis.txt

    write_result(cpu_time_used,n);
    printf("multiplication took %f seconds to execute \n", cpu_time_used); //mostrar el tiempo
    if(doPrint != 0){
        printResult(result,n);
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