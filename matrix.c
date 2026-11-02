#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

#define S 20           // Matrix size (20 x 20)
#define NUM_THREADS 10 // Total number of threads

int matA[S][S];
int matB[S][S];
int matSumResult[S][S];
int matDiffResult[S][S];
int matProductResult[S][S];

// Structure to pass thread arguments
typedef struct {
    int thread_id;
    int start_row;
    int end_row;
} thread_args_t;

// Fill matrix with random values
void fillMatrix(int matrix[S][S]) {
    for (int i = 0; i < S; i++) {
        for (int j = 0; j < S; j++) {
            matrix[i][j] = rand() % 10;
        }
    }
}

// Print matrix
void printMatrix(int matrix[S][S]) {
    for (int i = 0; i < S; i++) {
        for (int j = 0; j < S; j++) {
            printf("%5d ", matrix[i][j]);
        }
        printf("\n");
    }
    printf("\n");
}

// Thread function for Matrix Addition
void* matrixAdd(void* args) {
    thread_args_t* targs = (thread_args_t*)args;
    for (int i = targs->start_row; i < targs->end_row; i++) {
        for (int j = 0; j < S; j++) {
            matSumResult[i][j] = matA[i][j] + matB[i][j];
        }
    }
    pthread_exit(NULL);
}

// Thread function for Matrix Subtraction
void* matrixSubtract(void* args) {
    thread_args_t* targs = (thread_args_t*)args;
    for (int i = targs->start_row; i < targs->end_row; i++) {
        for (int j = 0; j < S; j++) {
            matDiffResult[i][j] = matA[i][j] - matB[i][j];
        }
    }
    pthread_exit(NULL);
}

// Thread function for Matrix Dot Product (Multiplication)
void* matrixMultiply(void* args) {
    thread_args_t* targs = (thread_args_t*)args;
    for (int i = targs->start_row; i < targs->end_row; i++) {
        for (int j = 0; j < S; j++) {
            matProductResult[i][j] = 0;
            for (int k = 0; k < S; k++) {
                matProductResult[i][j] += matA[i][k] * matB[k][j];
            }
        }
    }
    pthread_exit(NULL);
}

int main() {
    srand(time(NULL));

    // Initialize matrices A and B with random numbers
    fillMatrix(matA);
    fillMatrix(matB);

    printf("Matrix A:\n");
    printMatrix(matA);

    printf("Matrix B:\n");
    printMatrix(matB);

    pthread_t threads[NUM_THREADS];
    thread_args_t args[NUM_THREADS];
    int rows_per_thread = S / NUM_THREADS; // 20 / 10 = 2 rows per thread

    // --- 1. PARALLEL MATRIX ADDITION ---
    for (int i = 0; i < NUM_THREADS; i++) {
        args[i].thread_id = i;
        args[i].start_row = i * rows_per_thread;
        args[i].end_row = (i + 1) * rows_per_thread;
        pthread_create(&threads[i], NULL, matrixAdd, &args[i]);
    }
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    printf("Result: Matrix Addition\n");
    printMatrix(matSumResult);

    // --- 2. PARALLEL MATRIX SUBTRACTION ---
    for (int i = 0; i < NUM_THREADS; i++) {
        args[i].start_row = i * rows_per_thread;
        args[i].end_row = (i + 1) * rows_per_thread;
        pthread_create(&threads[i], NULL, matrixSubtract, &args[i]);
    }
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    printf("Result: Matrix Subtraction\n");
    printMatrix(matDiffResult);

    // --- 3. PARALLEL MATRIX DOT PRODUCT ---
    for (int i = 0; i < NUM_THREADS; i++) {
        args[i].start_row = i * rows_per_thread;
        args[i].end_row = (i + 1) * rows_per_thread;
        pthread_create(&threads[i], NULL, matrixMultiply, &args[i]);
    }
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    printf("Result: Matrix Dot Product (Multiplication)\n");
    printMatrix(matProductResult);

    return 0;
}