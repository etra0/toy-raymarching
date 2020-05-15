GXX=gcc
INCLUDE=-Iinclude/ -I/usr/include/SDL2/
LINK=-lSDL2main -lSDL2

main: src/main.c bin/rays.o
	$(GXX) -g -o $@  $? $(INCLUDE) $(LINK)

parallel: src/main.c bin/rays.o bin/parallel_render.o
	$(GXX) -g -o $@ -DPARALLEL $? $(INCLUDE) $(LINK) -lpthread


bin/rays.o: src/rays.c
	$(GXX) -c -o $@ $? $(INCLUDE)

bin/parallel_render.o: src/parallel_render.c
	$(GXX) -c -o $@ $? $(INCLUDE)

