all: my_route_lookup

my_route_lookup: main.c io.c
	gcc -Wall main.c io.c -o my_route_lookup -lm

.PHONY: clean

clean:
	rm -f my_route_lookup
