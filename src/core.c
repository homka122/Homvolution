#include <ctype.h>
#include <homv_matrix.h>
#include <libgen.h>
#include <omp.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include "homv_core.h"
#include "queue.h"
#include "stb_image.h"
#include "stb_image_write.h"

// clang-format off
double matrix_sharpen_values[] = {
  0.0, -1.0, 0.0,
  -1.0, 4.0, -1.0,
  0.0, -1.0, 0.0
};

double matrix_blur_values[] = {
  1.0/9.0, 1.0/9.0, 1.0/9.0,
  1.0/9.0, 1.0/9.0, 1.0/9.0,
  1.0/9.0, 1.0/9.0, 1.0/9.0
};

double matrix_identity_values[] = {
  0, 0, 0,
  0, 1, 0,
  0, 0, 0
};

double matrix_bottom_sobel[] = {
	-1, -2, -1,
	0, 0, 0,
	1, 2, 1
};

double matrix_outline[] = {
	-1, -1, -1,
	-1, 8, -1,
	-1, -1, -1
};
// clang-format on

homv_matrix homv_matrices[HOMV_MATRIX_MAX] = {
    [HOMV_MATRIX_SHARPEN] = {.size = 3, .values = matrix_sharpen_values},
    [HOMV_MATRIX_BLUR] = {.size = 3, .values = matrix_blur_values},
    [HOMV_MATRIX_IDENTITY] = {.size = 3, .values = matrix_identity_values},
    [HOMV_MATRIX_BOTTOM_SOBEL] = {.size = 3, .values = matrix_bottom_sobel},
    [HOMV_MATRIX_OUTLINE] = {.size = 3, .values = matrix_outline},
};

ssize_t area_width = 0;
ssize_t area_height = 0;

char *filenames[FILE_NAMES_MAX_COUNT];
size_t filenames_count;

uint8_t *homv_apply_seq(const uint8_t *image_input, int width, int height, int channels, homv_matrix matrix_input) {
  uint8_t *output = calloc(width * height * channels, sizeof(uint8_t));

  ssize_t mx_size = ((ssize_t)matrix_input.size);
  for (ssize_t img_x = 0; img_x < width; img_x++) {
    for (ssize_t img_y = 0; img_y < height; img_y++) {
      ssize_t input_img_x = img_x + mx_size / 2;
      ssize_t input_img_y = img_y + mx_size / 2;
      for (ssize_t mx_x = 0; mx_x < mx_size; mx_x++) {
        for (ssize_t mx_y = 0; mx_y < mx_size; mx_y++) {
          ssize_t mx_img_x = input_img_x + (mx_x - (mx_size / 2));
          ssize_t mx_img_y = input_img_y + (mx_y - (mx_size / 2));
          for (ssize_t color = 0; color < channels; color++) {
            output[img_y * width * channels + img_x * channels + color] +=
                image_input[mx_img_y * (width + mx_size - 1) * channels + mx_img_x * channels + color] *
                matrix_input.values[mx_y * mx_size + mx_x];
          }
        }
      }
    }
  }

  return (uint8_t *)output;
}

uint8_t *homv_apply_parallel_rows(const uint8_t *image_input, int width, int height, int channels,
                                  homv_matrix matrix_input) {
  uint8_t *output = calloc(width * height * channels, sizeof(uint8_t));

  ssize_t mx_size = ((ssize_t)matrix_input.size);
  ssize_t img_x, img_y, mx_x, mx_y, color;
#pragma omp parallel for shared(output) private(img_x, img_y, mx_x, mx_y, color)
  for (img_y = 0; img_y < height; img_y++) {
    for (img_x = 0; img_x < width; img_x++) {
      ssize_t input_img_x = img_x + mx_size / 2;
      ssize_t input_img_y = img_y + mx_size / 2;
      for (mx_x = 0; mx_x < mx_size; mx_x++) {
        for (mx_y = 0; mx_y < mx_size; mx_y++) {
          ssize_t mx_img_x = input_img_x + (mx_x - (mx_size / 2));
          ssize_t mx_img_y = input_img_y + (mx_y - (mx_size / 2));
          for (color = 0; color < channels; color++) {
            output[img_y * width * channels + img_x * channels + color] +=
                image_input[mx_img_y * (width + mx_size - 1) * channels + mx_img_x * channels + color] *
                matrix_input.values[mx_y * mx_size + mx_x];
          }
        }
      }
    }
  }

  return (uint8_t *)output;
}

uint8_t *homv_apply_parallel_cols(const uint8_t *image_input, int width, int height, int channels,
                                  homv_matrix matrix_input) {
  uint8_t *output = calloc(width * height * channels, sizeof(uint8_t));

  ssize_t mx_size = ((ssize_t)matrix_input.size);
  ssize_t img_x, img_y, mx_x, mx_y, color;
#pragma omp parallel for shared(output) private(img_x, img_y, mx_x, mx_y, color)
  for (img_x = 0; img_x < width; img_x++) {
    for (img_y = 0; img_y < height; img_y++) {
      ssize_t input_img_x = img_x + mx_size / 2;
      ssize_t input_img_y = img_y + mx_size / 2;
      for (mx_x = 0; mx_x < mx_size; mx_x++) {
        for (mx_y = 0; mx_y < mx_size; mx_y++) {
          ssize_t mx_img_x = input_img_x + (mx_x - (mx_size / 2));
          ssize_t mx_img_y = input_img_y + (mx_y - (mx_size / 2));
          for (color = 0; color < channels; color++) {
            output[img_y * width * channels + img_x * channels + color] +=
                image_input[mx_img_y * (width + mx_size - 1) * channels + mx_img_x * channels + color] *
                matrix_input.values[mx_y * mx_size + mx_x];
          }
        }
      }
    }
  }

  return (uint8_t *)output;
}

uint8_t *homv_apply_parallel_pixels(const uint8_t *image_input, int width, int height, int channels,
                                    homv_matrix matrix_input) {
  uint8_t *output = calloc(width * height * channels, sizeof(uint8_t));

  ssize_t mx_size = ((ssize_t)matrix_input.size);
  ssize_t pixel_id, mx_x, mx_y, color;
#pragma omp parallel for shared(output) private(pixel_id, mx_x, mx_y, color)
  for (pixel_id = 0; pixel_id < width * height; pixel_id++) {
    ssize_t img_x = pixel_id % width;
    ssize_t img_y = pixel_id / width;
    ssize_t input_img_x = img_x + mx_size / 2;
    ssize_t input_img_y = img_y + mx_size / 2;
    for (mx_x = 0; mx_x < mx_size; mx_x++) {
      for (mx_y = 0; mx_y < mx_size; mx_y++) {
        ssize_t mx_img_x = input_img_x + (mx_x - (mx_size / 2));
        ssize_t mx_img_y = input_img_y + (mx_y - (mx_size / 2));
        for (color = 0; color < channels; color++) {
          output[img_y * width * channels + img_x * channels + color] +=
              image_input[mx_img_y * (width + mx_size - 1) * channels + mx_img_x * channels + color] *
              matrix_input.values[mx_y * mx_size + mx_x];
        }
      }
    }
  }

  return (uint8_t *)output;
}

// global variables for type compability
uint8_t *homv_apply_parallel_area(const uint8_t *image_input, int width, int height, int channels,
                                  homv_matrix matrix_input) {
  uint8_t *output = calloc(width * height * channels, sizeof(uint8_t));
  ssize_t mx_size = ((ssize_t)matrix_input.size);

  ssize_t img_x, img_y, mx_x, mx_y, color;
  ssize_t area_row_counts = (height + area_height - 1) / area_height; // module with ceil
  ssize_t area_col_counts = (width + area_width - 1) / area_width;
  ssize_t area_index = 0;
#pragma omp parallel for shared(output) private(area_index, img_x, img_y, mx_x, mx_y, color)
  for (area_index = 0; area_index < area_row_counts * area_col_counts; area_index++) {
    ssize_t area_x = area_index % area_col_counts;
    ssize_t area_y = area_index / area_col_counts;
    for (img_y = area_y * area_height; img_y < (area_y + 1) * area_height && img_y < height; img_y++) {
      for (img_x = area_x * area_width; img_x < (area_x + 1) * area_width && img_x < width; img_x++) {
        ssize_t input_img_x = img_x + mx_size / 2;
        ssize_t input_img_y = img_y + mx_size / 2;
        for (mx_x = 0; mx_x < mx_size; mx_x++) {
          for (mx_y = 0; mx_y < mx_size; mx_y++) {
            ssize_t mx_img_x = input_img_x + (mx_x - (mx_size / 2));
            ssize_t mx_img_y = input_img_y + (mx_y - (mx_size / 2));
            for (color = 0; color < channels; color++) {
              output[img_y * width * channels + img_x * channels + color] +=
                  image_input[mx_img_y * (width + mx_size - 1) * channels + mx_img_x * channels + color] *
                  matrix_input.values[mx_y * mx_size + mx_x];
            }
          }
        }
      }
    }
  }

  return (uint8_t *)output;
}

// Resize image by reflecting edges of images
// If we have image
// [1 2 3]
// [4 5 6]
// [7 8 9]
// And kernel by 3x3 we must resize image to 5x5 size
// One option is use reflecting and get new image:
// [5 4 5 6 5]
// [2 1 2 3 2]
// [5 4 5 6 5]
// [8 7 8 9 8]
// [5 4 5 6 5]
// After that we can process image without checking borders
uint8_t *homv_reflect_image(uint8_t *old_image, int width, int height, int channels, size_t kernel_size) {
  if (kernel_size % 2 != 1) {
    fprintf(stderr, "Kernel size must be odd number\n");
    return NULL;
  }

  ssize_t new_width = (ssize_t)(width + (kernel_size - 1));
  ssize_t new_heigth = (ssize_t)(height + (kernel_size - 1));
  uint8_t *new_image = malloc(new_width * new_heigth * channels * sizeof(uint8_t));

  // idea is reflect index
  // if we have image 2x2:
  // [(0 0) (0 1)]
  // [(1 0) (1 1)]
  // After add 1-size padding we get image with there coordinates
  // [(-1 -1) (-1 0) (-1 1) (-1 2)]
  // [( 0 -1) ( 0 0) ( 0 1) ( 0 2)]
  // [( 1 -1) ( 1 0) ( 1 1) ( 1 2)]
  // [( 2 -1) ( 2 0) ( 2 1) ( 2 2)]
  // if coordinate below zero we just multiply by -1 for reflected index (-1 -1) -> (1 1)
  // if coordinate above max coordinate of initial image we must subtract double difference (2 2) -> (0 0)
  for (ssize_t col = -(kernel_size / 2); col < (ssize_t)(width + (kernel_size / 2)); col++) {
    for (ssize_t row = -(kernel_size / 2); row < (ssize_t)(height + kernel_size / 2); row++) {
      for (ssize_t color = 0; color < channels; color++) {
        ssize_t reflected_col;
        if (col < 0) {
          reflected_col = -col;
        } else if (col < width) {
          reflected_col = col;
        } else {
          reflected_col = 2 * width - col - 2;
        }

        ssize_t reflected_row;
        if (row < 0) {
          reflected_row = -row;
        } else if (row < height) {
          reflected_row = row;
        } else {
          reflected_row = 2 * height - row - 2;
        }

        new_image[(row + kernel_size / 2) * new_width * channels + (col + kernel_size / 2) * channels + color] =
            old_image[reflected_row * width * channels + reflected_col * channels + color];
      }
    }
  }

  return new_image;
}

#define READERS_COUNT 3
#define WORKERS_COUNT 6
#define WRITERS_COUNT 3

pthread_mutex_t queue_mutex;
pthread_t readers[READERS_COUNT], workers[WORKERS_COUNT], writers[WRITERS_COUNT];
queue_t *queue_readers;
queue_t *queue_workers;
queue_t *queue_writers;
homv_apply_type *method;
homv_matrix matrix;
size_t read_count, work_count;
bool read_ready = false, write_ready = false;

typedef struct {
  uint8_t *image;
  char *filename;
  int width;
  int height;
  int channels;
} node_image_data;

void *thread_func_reader(void *params_input) {
  (void)params_input;

  while (1) {
    pthread_mutex_lock(&queue_mutex);
    if (queue_readers->size == 0) {
      pthread_mutex_unlock(&queue_mutex);
      return NULL;
    }
    char *filename = queue_pop(queue_readers);
    pthread_mutex_unlock(&queue_mutex);

    int width, height, channels;
    uint8_t *img = stbi_load(filename, &width, &height, &channels, 0);

    if (!img) {
      printf("Failed to load image: %s\n", filename);
      return NULL;
    }

    printf("Loaded image: %dx%d, Channels: %d\n", width, height, channels);

    node_image_data *data = malloc(sizeof(node_image_data));
    data->image = img;
    data->filename = filename;
    data->width = width;
    data->height = height;
    data->channels = channels;
    pthread_mutex_lock(&queue_mutex);
    queue_add(queue_workers, (void *)data);
    if (queue_readers->size == 0) {
      read_ready = true;
    }
    pthread_mutex_unlock(&queue_mutex);
  }
}

void *thread_func_worker(void *params_input) {
  (void)params_input;

  while (1) {
    pthread_mutex_lock(&queue_mutex);
    if (queue_workers->size == 0) {
      if (queue_readers->size == 0 && read_ready) {
        pthread_mutex_unlock(&queue_mutex);
        return NULL;
      }

      pthread_mutex_unlock(&queue_mutex);
      continue;
    }
    node_image_data *data = queue_pop(queue_workers);
    pthread_mutex_unlock(&queue_mutex);

    uint8_t *image_reflected = homv_reflect_image(data->image, data->width, data->height, data->channels, matrix.size);
    uint8_t *output = method(image_reflected, data->width, data->height, data->channels, matrix);
    printf("Convolution applied to %s\n", data->filename);

    data->image = output;
    pthread_mutex_lock(&queue_mutex);
    queue_add(queue_writers, (void *)data);
    if (queue_workers->size == 0) {
      write_ready = true;
    }
    pthread_mutex_unlock(&queue_mutex);
  }
}

void *thread_func_writer(void *params_input) {
  (void)params_input;

  while (1) {
    pthread_mutex_lock(&queue_mutex);
    // printf("%ld %ld %ld\n", queue_readers->size, queue_workers->size, queue_writers->size);
    if (queue_writers->size == 0) {
      if (queue_readers->size == 0 && queue_workers->size == 0 && read_ready && write_ready) {
        pthread_mutex_unlock(&queue_mutex);
        return NULL;
      }

      pthread_mutex_unlock(&queue_mutex);
      continue;
    }
    node_image_data *data = queue_pop(queue_writers);
    pthread_mutex_unlock(&queue_mutex);

    char *filename = basename(data->filename);
    char *newfilename = malloc(sizeof(char) * (strlen(filename) + strlen("output/output_") + 1));

    newfilename[0] = '\0';
    strcat(newfilename, "output/output_");
    strcat(newfilename, filename);

    if (stbi_write_jpg(newfilename, data->width, data->height, data->channels, data->image, 100)) {
      printf("Image saved as %s\n", newfilename);
    } else {
      printf("Failed to save image\n");
      printf("%s, %d, %d, %d", newfilename, data->width, data->height, data->channels);
    }
  }
}

void queue_exec(char *filenames[FILE_NAMES_MAX_COUNT], size_t filenames_count, homv_apply_type method_input,
                homv_matrix matrix_input) {
  method = method_input;
  matrix = matrix_input;
  read_count = filenames_count;
  work_count = 0;

  pthread_mutex_init(&queue_mutex, NULL);

  queue_readers = queue_init();
  queue_workers = queue_init();
  queue_writers = queue_init();

  for (size_t i = 0; i < filenames_count; i++) {
    queue_add(queue_readers, (void *)filenames[i]);
  }

  for (size_t i = 0; i < READERS_COUNT; i++) {
    pthread_create(&readers[i], NULL, thread_func_reader, NULL);
  }
  for (size_t i = 0; i < WORKERS_COUNT; i++) {
    pthread_create(&workers[i], NULL, thread_func_worker, NULL);
  }
  for (size_t i = 0; i < WRITERS_COUNT; i++) {
    pthread_create(&writers[i], NULL, thread_func_writer, NULL);
  }

  for (size_t i = 0; i < READERS_COUNT; i++) {
    void *result;
    pthread_join(readers[i], &result);
  }

  for (size_t i = 0; i < WORKERS_COUNT; i++) {
    void *result;
    pthread_join(workers[i], &result);
  }

  for (size_t i = 0; i < WRITERS_COUNT; i++) {
    void *result;
    pthread_join(writers[i], &result);
  }

  queue_free(queue_readers);
  queue_free(queue_workers);
  queue_free(queue_writers);

  return;
}
