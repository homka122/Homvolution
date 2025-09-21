
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

// clang-format off
#include <setjmp.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <cmocka.h>
// clang-format on

#include <stdio.h>
#include <sys/types.h>

#include "homv_core.h"
#include "homv_matrix.h"
#include "stb_image.h"
#include "stb_image_write.h"

#define LOAD_IMAGE(path)                                                   \
	int width, height, channels;                                             \
	homv_matrix matrix = (homv_matrix){.size = 3, .values = matrix_outline}; \
	uint8_t *img = stbi_load(path, &width, &height, &channels, 0);           \
	if (!img) {                                                              \
		printf("Failed to load image\n");                                      \
		exit(-1);                                                              \
	}                                                                        \
	uint8_t *image_reflected = homv_reflect_image(img, width, height, channels, matrix.size);

#define APPLY_METHOD(name)                                                                  \
	uint8_t *first_output = homv_apply_seq(image_reflected, width, height, channels, matrix); \
	uint8_t *second_output = name(image_reflected, width, height, channels, matrix);

#define FREE_WORKSPACE() \
	free(img);             \
	free(image_reflected); \
	free(first_output);    \
	free(second_output);

static void test_rows_method(void **state) {
	(void)state;

	LOAD_IMAGE("./input/sticker.jpg");

	APPLY_METHOD(homv_apply_parallel_rows);

	for (ssize_t i = 0; i < width * height * channels; i++) {
		assert_int_equal(first_output[i], second_output[i]);
	}

	FREE_WORKSPACE();
}

static void test_cols_method(void **state) {
	(void)state;

	LOAD_IMAGE("./input/sticker.jpg");

	APPLY_METHOD(homv_apply_parallel_cols);

	for (ssize_t i = 0; i < width * height * channels; i++) {
		assert_int_equal(first_output[i], second_output[i]);
	}

	FREE_WORKSPACE();
}

static void test_pixels_method(void **state) {
	(void)state;

	LOAD_IMAGE("./input/sticker.jpg");

	APPLY_METHOD(homv_apply_parallel_pixels);

	for (ssize_t i = 0; i < width * height * channels; i++) {
		assert_int_equal(first_output[i], second_output[i]);
	}

	FREE_WORKSPACE();
}

extern ssize_t area_width;
extern ssize_t area_height;
static void test_area_method(void **state) {
	(void)state;

	LOAD_IMAGE("./input/sticker.jpg");

	area_height = 3;
	area_width = 2;

	APPLY_METHOD(homv_apply_parallel_area);

	for (ssize_t i = 0; i < width * height * channels; i++) {
		assert_int_equal(first_output[i], second_output[i]);
	}

	FREE_WORKSPACE();
}

int main(void) {
	const struct CMUnitTest tests[] = {
			cmocka_unit_test(test_rows_method),
			cmocka_unit_test(test_cols_method),
			cmocka_unit_test(test_pixels_method),
			cmocka_unit_test(test_area_method),
	};

	return cmocka_run_group_tests(tests, NULL, NULL);
}