#include <ctype.h>
#include <homv_matrix.h>
#include <libgen.h>
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include "homv_core.h"
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
			for (ssize_t mx_x = 0; mx_x < mx_size; mx_x++) {
				for (ssize_t mx_y = 0; mx_y < mx_size; mx_y++) {
					for (ssize_t color = 0; color < channels; color++) {
						ssize_t mx_img_x = img_x + (mx_x - (mx_size / 2));
						ssize_t mx_img_y = img_y + (mx_y - (mx_size / 2));
						if (mx_img_x < 0 || mx_img_x >= width || mx_img_y < 0 || mx_img_y >= height) continue;

						output[img_y * width * channels + img_x * channels + color] +=
								image_input[mx_img_y * width * channels + (mx_img_x)*channels + color] *
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
			for (mx_x = 0; mx_x < mx_size; mx_x++) {
				for (mx_y = 0; mx_y < mx_size; mx_y++) {
					for (color = 0; color < channels; color++) {
						ssize_t mx_img_x = img_x + (mx_x - (mx_size / 2));
						ssize_t mx_img_y = img_y + (mx_y - (mx_size / 2));
						if (mx_img_x < 0 || mx_img_x >= width || mx_img_y < 0 || mx_img_y >= height) continue;

						output[img_y * width * channels + img_x * channels + color] +=
								image_input[mx_img_y * width * channels + (mx_img_x)*channels + color] *
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
			for (mx_x = 0; mx_x < mx_size; mx_x++) {
				for (mx_y = 0; mx_y < mx_size; mx_y++) {
					for (color = 0; color < channels; color++) {
						ssize_t mx_img_x = img_x + (mx_x - (mx_size / 2));
						ssize_t mx_img_y = img_y + (mx_y - (mx_size / 2));
						if (mx_img_x < 0 || mx_img_x >= width || mx_img_y < 0 || mx_img_y >= height) continue;

						output[img_y * width * channels + img_x * channels + color] +=
								image_input[mx_img_y * width * channels + (mx_img_x)*channels + color] *
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
		for (mx_x = 0; mx_x < mx_size; mx_x++) {
			for (mx_y = 0; mx_y < mx_size; mx_y++) {
				for (color = 0; color < channels; color++) {
					ssize_t img_x = pixel_id % width;
					ssize_t img_y = pixel_id / width;

					ssize_t mx_img_x = img_x + (mx_x - (mx_size / 2));
					ssize_t mx_img_y = img_y + (mx_y - (mx_size / 2));
					if (mx_img_x < 0 || mx_img_x >= width || mx_img_y < 0 || mx_img_y >= height) continue;

					output[img_y * width * channels + img_x * channels + color] +=
							image_input[mx_img_y * width * channels + (mx_img_x)*channels + color] *
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
	ssize_t area_row_counts = (height + area_height - 1) / area_height;	 // module with ceil
	ssize_t area_col_counts = (width + area_width - 1) / area_width;
	ssize_t area_index = 0;
#pragma omp parallel for shared(output) private(area_index, img_x, img_y, mx_x, mx_y, color)
	for (area_index = 0; area_index < area_row_counts * area_col_counts; area_index++) {
		ssize_t area_x = area_index % area_col_counts;
		ssize_t area_y = area_index / area_col_counts;
		for (img_y = area_y * area_height; img_y < (area_y + 1) * area_height && img_y < height; img_y++) {
			for (img_x = area_x * area_width; img_x < (area_x + 1) * area_width && img_x < width; img_x++) {
				for (mx_x = 0; mx_x < mx_size; mx_x++) {
					for (mx_y = 0; mx_y < mx_size; mx_y++) {
						for (color = 0; color < channels; color++) {
							ssize_t mx_img_x = img_x + (mx_x - (mx_size / 2));
							ssize_t mx_img_y = img_y + (mx_y - (mx_size / 2));
							if (mx_img_x < 0 || mx_img_x >= width || mx_img_y < 0 || mx_img_y >= height) continue;

							output[img_y * width * channels + img_x * channels + color] +=
									image_input[mx_img_y * width * channels + (mx_img_x)*channels + color] *
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