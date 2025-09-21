#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include <math.h>
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "homv_core.h"
#include "homv_matrix.h"
#include "stb_image.h"
#include "stb_image_write.h"

#define NUM_RUNS 40

typedef struct {
  double times[NUM_RUNS];
  size_t count;
} bench_results;

double average(bench_results *res) {
  double sum = 0;
  for (size_t i = 0; i < res->count; i++)
    sum += res->times[i];
  return sum / res->count;
}

void run_benchmark(uint8_t *img, int width, int height, int channels, homv_matrix matrix, homv_apply_type method,
                   bench_results *res) {
  for (size_t i = 0; i < NUM_RUNS; i++) {
    double start = omp_get_wtime();
    uint8_t *reflected_image = homv_reflect_image(img, width, height, channels, matrix.size);
    uint8_t *output = method(reflected_image, width, height, channels, matrix);
    double end = omp_get_wtime();
    res->times[i] = end - start;
    free(reflected_image);
    free(output);
  }
  res->count = NUM_RUNS;
}

int main(int argc, char **argv) {
  if (argc < 3) {
    printf("Usage: %s matrix (queue|noqueue) image1 [image2 ...]\n", argv[0]);
    return 1;
  }

  homv_matrix matrix;
  if (strcmp(argv[1], "sharpen") == 0) {
    matrix = homv_matrices[HOMV_MATRIX_SHARPEN];
  } else if (strcmp(argv[1], "blur") == 0) {
    matrix = homv_matrices[HOMV_MATRIX_BLUR];
  } else if (strcmp(argv[1], "identity") == 0) {
    matrix = homv_matrices[HOMV_MATRIX_IDENTITY];
  } else if (strcmp(argv[1], "bottom_sobel") == 0) {
    matrix = homv_matrices[HOMV_MATRIX_BOTTOM_SOBEL];
  } else if (strcmp(argv[1], "outline") == 0) {
    matrix = homv_matrices[HOMV_MATRIX_OUTLINE];
  } else if (strcmp(argv[1], "random") == 0) {
    matrix = *homv_mx_get_random_matrix(9);
  } else {
    fprintf(stderr, "Unknown matrix: %s\n", argv[1]);
    return 1;
  }
  printf("Chosen matrix: [%s]\n", argv[1]);

  bench_results seq, rows, cols, area;

  for (int i = 2; i < argc; i++) {
    int width, height, channels;
    uint8_t *img = stbi_load(argv[i], &width, &height, &channels, 0);
    if (!img) {
      fprintf(stderr, "Failed to load image %s\n", argv[i]);
      continue;
    }

    printf("\nImage %s (%dx%d)\n", argv[i], width, height);

    run_benchmark(img, width, height, channels, matrix, homv_apply_seq, &seq);
    printf("Seq: %.4f", seq.times[0]);
    for (size_t i = 1; i < seq.count; i++) {
      printf(",%.4f", seq.times[i]);
    }
    printf("\n");

    run_benchmark(img, width, height, channels, matrix, homv_apply_parallel_rows, &rows);
    printf("Rows: %.4f", rows.times[0]);
    for (size_t i = 1; i < rows.count; i++) {
      printf(",%.4f", rows.times[i]);
    }
    printf("\n");

    run_benchmark(img, width, height, channels, matrix, homv_apply_parallel_cols, &cols);
    printf("Cols: %.4f", cols.times[0]);
    for (size_t i = 1; i < cols.count; i++) {
      printf(",%.4f", cols.times[i]);
    }
    printf("\n");

    area_width = area_height = 4;
    run_benchmark(img, width, height, channels, matrix, homv_apply_parallel_area, &area);
    printf("Area(4x4): %.4f", area.times[0]);
    for (size_t i = 1; i < area.count; i++) {
      printf(",%.4f", area.times[i]);
    }
    printf("\n");

    stbi_image_free(img);
  }

  return 0;
}
