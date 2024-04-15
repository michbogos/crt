make: main.c
	gcc main.c -o crt -lm -Ofast

run: main.c
	gcc main.c -o crt -lm -Ofast -Wall -Wextra && ./crt > img.ppm && open img.ppm