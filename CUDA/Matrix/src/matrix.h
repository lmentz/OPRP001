#ifndef __MATRIX_H
#define __MATRIX_H

#include <stdlib.h>

#define random() ((rand() ^ rand()) / (RAND_MAX + 1.0))

typedef struct {
  double **data;
  int rows;
  int cols;
} matrix_t;

typedef struct {
  double *A;
  double *B;
  double *C;
  int len;
} matrix_sum_args_t;

typedef struct {
  matrix_t *A;
  matrix_t *B;
  matrix_t *C;
  int i0, i1;
} matrix_mult_args_t;

/*
 * All functions must return a new matriz (when need)
 */

matrix_t *matrix_create(int rows, int cols);
void matrix_destroy(matrix_t *m);
void matrix_randfill(matrix_t *m);
void matrix_fill(matrix_t *m, double val);
matrix_t *matrix_multiply(matrix_t *A, matrix_t *B, matrix_t *ret);
matrix_t *matrix_sum(matrix_t *A, matrix_t *B, matrix_t *ret);
matrix_t *matrix_sort(matrix_t *A, matrix_t *ret);
void matrix_print(matrix_t *m);

// Métodos com uso de GPU
matrix_t *matrix_multiply_gpu(matrix_t *A, matrix_t *B, matrix_t *ret);
matrix_t *matrix_sum_gpu(matrix_t *A, matrix_t *B, matrix_t *ret);

#endif
