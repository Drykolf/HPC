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
            mat[i][j] = i+j;
        }
    }
    return mat;
}

void write_time_taken(double time, int data) {
    FILE *f = fopen("resultados.txt", "a");
    if (f == NULL) {
        printf("Error opening file!\n");
        exit(1);
    }
    fprintf(f, "Tiempo consumido: %f, para cantidad: %d\n", time, data);
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
    clock_t start, end;
    double cpu_time_used;

    initialize_matrices(&matrixA, &matrixB, &result, n, &shmid);

    start = clock();
    multiply_matrices_processes(matrixA, matrixB, result, n);
    end = clock();

    cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
    write_time_taken(cpu_time_used, n);
    printf("multiplication took %f seconds to execute \n", cpu_time_used);

    if (doPrint != 0) {
        print_result(result, n);
    }

    free_matrices(matrixA, matrixB, result, n, shmid);

    return 0;
}