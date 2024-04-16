make: main.c
	gcc main.c -o crt -lm -Ofast

run: main.c
	gcc main.c -o crt -lm -Wall -Wextra && ./crt > img.ppm && open img.ppm

debug: main.c
	gcc main.c -o crt -lm -g -Wall -Wextra