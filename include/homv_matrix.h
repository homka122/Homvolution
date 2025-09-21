#ifndef HOMV_MATRIX_H
#define HOMV_MATRIX_H

#include <inttypes.h>
#include <stdlib.h>

// Kernel for image processing.
// Matrix must be is square.
typedef struct {
  size_t size;
  double *values; // size x size array of values of matrix
} homv_matrix;

homv_matrix *homv_mx_init(size_t size, double **input);
void homv_mx_free(homv_matrix *mx);
homv_matrix *homv_mx_get_random_matrix(size_t size);

#endif
