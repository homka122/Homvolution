#include "homv_matrix.h"
#include <stdio.h>

homv_matrix *homv_mx_init(size_t size, double **input) {
  if (input != NULL) {
    fprintf(stderr, "Values for matrix don't accepting now\n");
    exit(-1);
  }

  homv_matrix *result = malloc(sizeof(homv_matrix));
  result->size = size;
  result->values = calloc(size * size, sizeof(double));

  return result;
}

void homv_mx_free(homv_matrix *mx) {
  free(mx->values);
  free(mx);

  return;
}

static double rand_double() { return (double)(rand()) / RAND_MAX - 0.5; }

homv_matrix *homv_mx_get_random_matrix(size_t size) {
  homv_matrix *result = homv_mx_init(size, NULL);

  for (size_t i = 0; i < size * size; i++) {
    result->values[i] = rand_double();
  }

  return result;
}
