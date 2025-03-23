#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/shm.h>

#define MAX_PROCCES 4

void multi(int** matA, int** matB, int* result, int size, int process_id) {
    for (int i = process_id * size / MAX_PROCCES; i < (process_id + 1) * size / MAX_PROCCES; i++) {
        for (int j = 0; j < size; j++) {
            result[i * size + j] = 0;
            for (int k = 0; k < size; k++) {
                result[i * size + j] += matA[i][k] * matB[k][j];
            }
        }
    }
}

void multiply_matrices_processes(int** matA, int** matB, int* result, int size) {
    pid_t pids[MAX_PROCCES];

    for (int i = 0; i < MAX_PROCCES; i++) {
        if ((pids[i] = fork()) == 0) {
            multi(matA, matB, result, size, i);
            exit(0);
        }
    }

    for (int i = 0; i < MAX_PROCCES; i++) {
        waitpid(pids[i], NULL, 0);
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
    char filename[50];
    sprintf(filename, "resultados_%d.txt", MAX_PROCCES);
    FILE *f = fopen(filename, "a");
    if (f == NULL) {
        printf("Error opening file!\n");
        exit(1);
    }
    fprintf(f, "Tiempo consumido: %.3f, para cantidad: %d\n", time, data);
    fclose(f);
}

void print_result(int* result, int size) {
    FILE *f = fopen("matriz.txt", "w");
    if (f == NULL) {
        printf("Error opening file!\n");
        exit(1);
    }
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            fprintf(f, "%d ", result[i * size + j]);
        }
        fprintf(f, "\n");
    }
    fclose(f);
}

void initialize_matrices(int*** matrixA, int*** matrixB, int** result, int size, int* shmid) {
    *matrixA = generate_matrix(size);
    *matrixB = generate_matrix(size);
    *shmid = shmget(IPC_PRIVATE, size * size * sizeof(int), IPC_CREAT | 0666);
    if (*shmid < 0) {
        perror("shmget");
        exit(1);
    }
    *result = (int*)shmat(*shmid, NULL, 0);
    if (*result == (int*)-1) {
        perror("shmat");
        exit(1);
    }
}

void free_matrices(int** matrixA, int** matrixB, int* result, int size, int shmid) {
    for (int i = 0; i < size; i++) {
        free(matrixA[i]);
        free(matrixB[i]);
    }
    free(matrixA);
    free(matrixB);
    shmdt(result);
    shmctl(shmid, IPC_RMID, NULL);
}

int main(int argc, char *argv[]) {
    int doPrint = 0;
    if (argc != 2) {
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
    int* result;
    int shmid;
    struct timespec start, end;
    double cpu_time_used,ms_time;

    initialize_matrices(&matrixA, &matrixB, &result, n, &shmid);
    clock_gettime(CLOCK_MONOTONIC, &start);
    multiply_matrices_processes(matrixA, matrixB, result, n);
    clock_gettime(CLOCK_MONOTONIC, &end);

    //cpu_time_used = (end.tv_sec - start.tv_sec) + ((double)(end.tv_nsec - start.tv_nsec)) / 1e9;
    ms_time = (end.tv_sec - start.tv_sec) * 1000 + ((double)(end.tv_nsec - start.tv_nsec)) / 1e6; // Miliseg
    write_result(ms_time,n);
    printf("multiplication took %.3f milliseconds to execute \n", ms_time); //mostrar el tiempo en ms

    if (doPrint != 0) {
        print_result(result, n);
    }

    free_matrices(matrixA, matrixB, result, n, shmid);

    return 0;
}