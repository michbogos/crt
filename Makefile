slow: main.c pcg_basic.o
	gcc main.c pcg_basic.o -o crt -lm -O2 && ./crt > img.ppm && open img.ppm

run: main.c pcg_basic.o
	gcc main.c pcg_basic.o -o crt -lm -Ofast -march=native -mtune=native -Wall -Wextra && ./crt > img.ppm && open img.ppm

parallel: main.c pcg_basic.o
	gcc main.c pcg_basic.o -o crt -lm -Ofast -march=native -mtune=native -Wall -Wextra -fopenmp && ./crt > img.ppm && open img.ppm

debug: main.c pcg_basic.o
	gcc main.c pcg_basic.o -o crt -lm -g -Wall -Wextra

debug-optimized: main.c pcg_basic.o
	gcc main.c pcg_basic.o -o crt -lm -O2 -g

blaze: main.c pcg_basic.o
	gcc main.c pcg_basic.o -o crt -lm -Ofast -march=native -mtune=native -fno-signed-zeros -fno-trapping-math -funroll-loops -Wall -Wextra && ./crt > Ofast-march-mtune-math-opt-unroll.ppm

release: main.c pcg_basic.o
	gcc main.c pcg_basic.o -o crt -lm -Ofast -march=native -mtune=native -fno-signed-zeros -fno-trapping-math -Wall -Wextra && ./crt > img.ppm

pcg_basic.o: pcg_basic.c pcg_basic.h