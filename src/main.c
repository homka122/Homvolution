#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <ctype.h>
#include <homv_matrix.h>
#include <libgen.h>
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include "stb_image.h"
#include "stb_image_write.h"

enum {
	HOMV_MATRIX_SHARPEN = 0,
	HOMV_MATRIX_BLUR,
	HOMV_MATRIX_IDENTITY,
	HOMV_MATRIX_BOTTOM_SOBEL,
	HOMV_MATRIX_OUTLINE,
	HOMV_MATRIX_MAX
};

// clang-format off
static double matrix_sharpen_values[] = {
  0.0, -1.0, 0.0,
  -1.0, 4.0, -1.0,
  0.0, -1.0, 0.0
};

static double matrix_blur_values[] = {
  1.0/9.0, 1.0/9.0, 1.0/9.0,
  1.0/9.0, 1.0/9.0, 1.0/9.0,
  1.0/9.0, 1.0/9.0, 1.0/9.0
};

static double matrix_identity_values[] = {
  0, 0, 0,
  0, 1, 0,
  0, 0, 0
};

static double matrix_bottom_sobel[] = {
	-1, -2, -1,
	0, 0, 0,
	1, 2, 1
};

static double matrix_outline[] = {
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

#define FILE_NAMES_MAX_COUNT 256
char *filenames[FILE_NAMES_MAX_COUNT];
size_t filenames_count;

void print_help_message(char **argv) { printf("Usage: %s -p [seq] -m [blur | sharpen | random] ...files\n", argv[0]); }

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
ssize_t area_width = 0;
ssize_t area_height = 0;
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

int process_command_line(int argc, char **argv, char **parallel_mode, char **chosen_matrix) {
	extern char *optarg;
	extern int optind, opterr, optopt;

	int p_flag = 0, m_flag = 0, err_flag = 0;
	int opt = -1;
	while ((opt = getopt(argc, argv, ":p:m:h")) != -1) {
		switch (opt) {
			case 'h':
				print_help_message(argv);
				return 0;
			case 'p':
				p_flag++;
				*parallel_mode = optarg;
				break;
			case 'm':
				m_flag++;
				*chosen_matrix = optarg;
				break;
			case ':': /* -p or -m without operand */
				fprintf(stderr, "Option -%c requires an operand\n", optopt);
				err_flag++;
				break;
			case '?':
				fprintf(stderr, "Unrecognized option: '-%c'\n", optopt);
				err_flag++;
				break;
		}
	}

	if (!p_flag || !m_flag) {
		fprintf(stderr, "Options -p and -m required\n");
		return 1;
	}

	if (err_flag) {
		print_help_message(argv);
		return 1;
	}

	for (; optind < argc; optind++) {
		filenames[filenames_count] = malloc(sizeof(char) * strlen(argv[optind]) + 1);
		strcpy(filenames[filenames_count++], argv[optind]);
	}

	return 0;
}

int main(int argc, char **argv) {
	srand(time(NULL));
	char *parallel_mode = NULL;
	char *chosen_matrix = NULL;
	if (process_command_line(argc, argv, &parallel_mode, &chosen_matrix)) {
		return 1;
	}

	// check parsing of command line
	for (size_t i = 0; i < filenames_count; i++) {
		printf("Filename: %s\n", filenames[i]);
	}

	uint8_t *(*method)(const uint8_t *, int, int, int, homv_matrix);
	printf("Chosen mode [%s]\n", parallel_mode);
	if (strcmp(parallel_mode, "seq") == 0) {
		method = homv_apply_seq;
	} else if (strcmp(parallel_mode, "rows") == 0) {
		method = homv_apply_parallel_rows;
	} else if (strcmp(parallel_mode, "cols") == 0) {
		method = homv_apply_parallel_cols;
	} else if (strcmp(parallel_mode, "pixels") == 0) {
		method = homv_apply_parallel_pixels;
	} else if (strncmp(parallel_mode, "area", 4) == 0) {
		strtok(parallel_mode, "_");
		area_width = atoi(strtok(NULL, "_"));
		area_height = atoi(strtok(NULL, "_"));
		method = homv_apply_parallel_area;
	} else {
		fprintf(stderr, "Unknown mode: %s\n", parallel_mode);
		return 1;
	}

	homv_matrix matrix;
	if (strcmp(chosen_matrix, "sharpen") == 0) {
		matrix = homv_matrices[HOMV_MATRIX_SHARPEN];
	} else if (strcmp(chosen_matrix, "blur") == 0) {
		matrix = homv_matrices[HOMV_MATRIX_BLUR];
	} else if (strcmp(chosen_matrix, "identity") == 0) {
		matrix = homv_matrices[HOMV_MATRIX_IDENTITY];
	} else if (strcmp(chosen_matrix, "bottom_sobel") == 0) {
		matrix = homv_matrices[HOMV_MATRIX_BOTTOM_SOBEL];
	} else if (strcmp(chosen_matrix, "outline") == 0) {
		matrix = homv_matrices[HOMV_MATRIX_OUTLINE];
	} else if (strcmp(chosen_matrix, "random") == 0) {
		matrix = *homv_mx_get_random_matrix(3);
	} else {
		fprintf(stderr, "Unknown matrix: %s\n", chosen_matrix);
		return 1;
	}
	printf("Chosen matrix: [%s]\n", chosen_matrix);

	for (size_t filename_i = 0; filename_i < filenames_count; filename_i++) {
		char *filepath = filenames[filename_i];
		char *filename = basename(filepath);

		int width, height, channels;
		uint8_t *img = stbi_load(filepath, &width, &height, &channels, 0);

		if (!img) {
			printf("Failed to load image: %s\n", filepath);
			continue;
		}

		printf("Loaded image: %dx%d, Channels: %d\n", width, height, channels);

		double start;
		double end;
		start = omp_get_wtime();
		uint8_t *output = method(img, width, height, channels, matrix);
		end = omp_get_wtime();
		printf("Work took %f seconds\n", end - start);
		// uint8_t *output = img;

		char *newfilename = malloc(sizeof(char) * (strlen(filename) + strlen("output/output_") + 1));

		newfilename[0] = '\0';
		strcat(newfilename, "output/output_");
		strcat(newfilename, filename);

		if (stbi_write_jpg(newfilename, width, height, channels, output, 100)) {
			printf("Image saved as %s\n", newfilename);
		} else {
			printf("Failed to save image\n");
		}

		// Освобождаем память
		stbi_image_free(img);
		free(newfilename);
		free(filenames[filename_i]);
		free(output);
	}

	return 0;
}
