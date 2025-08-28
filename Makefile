all: build

build/homv_matrix.o: src/homv_matrix.c include/homv_matrix.h include/homv_core.h
	gcc -Wall -Wpedantic -Wextra -c src/homv_matrix.c -I./deps -I./include -g -o build/homv_matrix.o

build/cli.o: src/cli.c include/homv_matrix.h include/homv_core.h
	gcc -Wall -Wpedantic -Wextra -c src/cli.c -I./deps -fopenmp -I./include -g -o build/cli.o

build/core.o: src/core.c include/homv_matrix.h include/homv_core.h
	gcc -Wall -Wpedantic -Wextra -c src/core.c -I./deps -fopenmp -I./include -g -o build/core.o

build-cli: build/cli.o build/homv_matrix.o build/core.o
	gcc -Wall -Wpedantic -Wextra build/core.o build/cli.o build/homv_matrix.o -fopenmp -lm -I./deps -I./include -g -o build/app

run: build
	./build/app -p seq -m sharpen input/example.jpg && code output/output_example.jpg