all: build

build/homv_matrix.o: src/homv_matrix.c include/homv_matrix.h
	gcc -Wall -Wpedantic -Wextra -c src/homv_matrix.c -I./deps -I./include -g -o build/homv_matrix.o

build/main.o: src/main.c include/homv_matrix.h
	gcc -Wall -Wpedantic -Wextra -c src/main.c -I./deps -fopenmp -I./include -g -o build/main.o

build: build/main.o build/homv_matrix.o 
	gcc -Wall -Wpedantic -Wextra build/main.o build/homv_matrix.o -fopenmp -lm -I./deps -I./include -g -o build/app

run: build
	./build/app -p seq -m sharpen input/example.jpg && code output/output_example.jpg