CC = gcc
FLAGS = -std=c99 \
		-Wall \
		-Wextra \
		-Werror \
		-Wpointer-arith \
		-Wcast-align \
		-Wno-unknown-pragmas


lib:
	$(CC) $(FLAGS) --shared -o post.so -O3 -fPIC post.c

test:
	$(CC) $(FLAGS) -o unit-tests ./post.c ./tests/munit.c ./tests/unit.c && ./unit-tests

clean:
	rm *.so *.o unit-tests
