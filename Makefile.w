GXX=g++.exe
INCLUDE=-Isdl2/include/SDL2/ -Iinclude/
LINK=-Lsdl2/lib/ -lmingw32 -lSDL2main -lSDL2

main.exe: src/main.c bin/rays.o
	$(GXX) -g -o $@  $? $(INCLUDE) $(LINK)

bin/rays.o: src/rays.c
	$(GXX) -c -o $@ $? $(INCLUDE)
