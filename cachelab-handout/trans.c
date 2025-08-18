/* 
 * trans.c - Matrix transpose B = A^T
 *
 * Each transpose function must have a prototype of the form:
 * void trans(int M, int N, int A[N][M], int B[M][N]);
 *
 * A transpose function is evaluated by counting the number of misses
 * on a 1KB direct mapped cache with a block size of 32 bytes.
 */ 
#include <stdio.h>
#include "cachelab.h"

int is_transpose(int M, int N, int A[N][M], int B[M][N]);

/* 
 * transpose_submit - This is the solution transpose function that you
 *     will be graded on for Part B of the assignment. Do not change
 *     the description string "Transpose submission", as the driver
 *     searches for that string to identify the transpose function to
 *     be graded. 
 */
char transpose_submit_desc[] = "Transpose submission";
void transpose_submit(int M, int N, int A[N][M], int B[M][N])
{
}

/* 
 * You can define additional transpose functions below. We've defined
 * a simple one below to help you get started. 
 */ 

/* 
 * trans - A simple baseline transpose function, not optimized for the cache.
 */
char trans_desc[] = "Simple row-wise scan transpose";
void trans(int M, int N, int A[N][M], int B[M][N])
{
    int i, j, tmp;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            tmp = A[i][j];
            B[j][i] = tmp;
        }
    }    

}

int min(int a, int b) {
    return (a<b) ? a : b;
}

char trans_desc_tile[] = "Tile transpose";
void trans_tile(int M, int N, int A[N][M], int B[M][N]) {
    int i, j, ii, jj, temp, diagonal;

    for (ii = 0; ii < N; ii += 8) {
        for (jj = 0; jj < M; jj += 8) {
            for (i = ii; i < min(ii+8, N); i++) {
                for (j = jj; j < min(jj+8, M); j++) {
                    if (i != j) {
                        B[j][i] = A[i][j];
                    } else {
                        temp = A[i][j];
                        diagonal = i;
                    }
                }
                if (ii == jj) {
                    B[diagonal][diagonal] = temp;
                }
            }
        }
    }
}

char trans_desc_unroll[] = "Unroll transpose";
void trans_unroll(int M, int N, int A[N][M], int B[M][N]) {
    int i, ii, jj, j, temp, diagonal;

    for (ii = 0; ii < N; ii += 8) {
        for (jj = 0; jj < M; jj += 8) {
            if (ii!=jj) {
                    for (i = ii; i < min(ii+8, N); i++) {
                    B[jj][i] = A[i][jj];
                    B[jj+1][i] = A[i][jj+1];
                    B[jj+2][i] = A[i][jj+2];
                    B[jj+3][i] = A[i][jj+3];
                    B[jj+4][i] = A[i][jj+4];
                    B[jj+5][i] = A[i][jj+5];
                    B[jj+6][i] = A[i][jj+6];
                    B[jj+7][i] = A[i][jj+7];
                }
            } else {
                for (i = ii; i < min(ii+8, N); i++) {
                    for (j = jj; j < min(jj+8, M); j++) {
                        if (i != j) {
                            B[j][i] = A[i][j];
                        } else {
                            temp = A[i][j];
                            diagonal = i;
                        }
                    }
                    B[diagonal][diagonal] = temp;
                }
            }
        }
    }
}

/*
 * registerFunctions - This function registers your transpose
 *     functions with the driver.  At runtime, the driver will
 *     evaluate each of the registered functions and summarize their
 *     performance. This is a handy way to experiment with different
 *     transpose strategies.
 */
void registerFunctions()
{
    /* Register your solution function */
    // registerTransFunction(transpose_submit, transpose_submit_desc); 

    /* Register any additional transpose functions */
    registerTransFunction(trans, trans_desc); 

    registerTransFunction(trans_tile, trans_desc_tile);
    
    registerTransFunction(trans_unroll, trans_desc_unroll); 

}

/* 
 * is_transpose - This helper function checks if B is the transpose of
 *     A. You can check the correctness of your transpose by calling
 *     it before returning from the transpose function.
 */
int is_transpose(int M, int N, int A[N][M], int B[M][N])
{
    int i, j;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; ++j) {
            if (A[i][j] != B[j][i]) {
                return 0;
            }
        }
    }
    return 1;
}

