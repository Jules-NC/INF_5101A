all:
	mkdir -p bin
	gcc -o ./bin/vector.o -c vector.c
	gcc -o ./bin/quadtree_realloc.o -c quadtree_realloc.c
	gcc -o ./bin/a.out ./bin/quadtree_realloc.o ./bin/vector.o -lm

run:
	./bin/a.out

clean:
	rm -f ./bin/*.o
