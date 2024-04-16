make: main.c
	gcc main.c -o crt -lm -O2

run: main.c
	gcc main.c -o crt -lm -O1 -march=native -mtune=native -Wall -Wextra && ./crt > img.ppm && open img.ppm

debug: main.c
	gcc main.c -o crt -lm -g -Wall -Wextra

blaze: main.c
	gcc main.c -o crt -lm -Ofast -march=native -mtune=native -fno-signed-zeros -fno-trapping-math -funroll-loops -Wall -Wextra && ./crt > Ofast-march-mtune-math-opt-unroll.ppm

release: main.c
	gcc main.c -o crt -lm -Ofast -march=native -mtune=native -fno-signed-zeros -fno-trapping-math -Wall -Wextra && ./crt > img.ppm