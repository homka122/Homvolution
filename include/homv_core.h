#ifndef HOMV_CORE_H
#define HOMV_CORE_H

#include <homv_matrix.h>

// clang-format off
extern double matrix_sharpen_values[];
extern double matrix_blur_values[];
extern double matrix_identity_values[];
extern double matrix_bottom_sobel[];
extern double matrix_outline[];
// clang-format on

enum {
	HOMV_MATRIX_SHARPEN = 0,
	HOMV_MATRIX_BLUR,
	HOMV_MATRIX_IDENTITY,
	HOMV_MATRIX_BOTTOM_SOBEL,
	HOMV_MATRIX_OUTLINE,
	HOMV_MATRIX_MAX
};

extern homv_matrix homv_matrices[HOMV_MATRIX_MAX];

#define FILE_NAMES_MAX_COUNT 256
extern char *filenames[FILE_NAMES_MAX_COUNT];
extern size_t filenames_count;

typedef uint8_t *(homv_apply_type)(const uint8_t *image_input, int width, int height, int channels,
																	 homv_matrix matrix_input);

homv_apply_type homv_apply_seq;
homv_apply_type homv_apply_parallel_rows;
homv_apply_type homv_apply_parallel_cols;
homv_apply_type homv_apply_parallel_pixels;
extern ssize_t area_width;
extern ssize_t area_height;
homv_apply_type homv_apply_parallel_area;

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
uint8_t *homv_reflect_image(uint8_t *old_image, int width, int height, int channels, size_t kernel_size);

#endif