GXX=gcc
INCLUDE=-Iinclude/ -I/usr/include/SDL2/ -DN_THREADS=4
LINK=-lSDL2main -lSDL2

main: src/main.c bin/rays.o
	$(GXX) -g -O3 -o $@  $? $(INCLUDE) $(LINK)

parallel: src/main.c bin/rays.o bin/parallel_render.o
	$(GXX) -g -O3 -o $@ -DPARALLEL $? $(INCLUDE) $(LINK) -lpthread


bin/rays.o: src/rays.c
	$(GXX) -c -O3 -o $@ $? $(INCLUDE)

bin/parallel_render.o: src/parallel_render.c
	$(GXX) -c -O3 -o $@ $? $(INCLUDE)

