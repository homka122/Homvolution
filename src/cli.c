#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <ctype.h>
#include <libgen.h>
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include "homv_core.h"
#include "homv_matrix.h"
#include "stb_image.h"
#include "stb_image_write.h"

// global variables for type compability
extern ssize_t area_width;
extern ssize_t area_height;
extern char *filenames[FILE_NAMES_MAX_COUNT];
extern size_t filenames_count;

void print_help_message(char **argv) { printf("Usage: %s -p [seq] -m [blur | sharpen | random] ...files\n", argv[0]); }

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
		matrix = *homv_mx_get_random_matrix(9);
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
		uint8_t *image_reflected = homv_reflect_image(img, width, height, channels, matrix.size);
		uint8_t *output = method(image_reflected, width, height, channels, matrix);
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
