GXX=nvcc.exe
FLAGS=-O2 -arch=sm_35
INCLUDE=-Ivendor/sdl2/include/SDL2/ -Iinclude/ -Ivendor/pthreads-win32/include/
LINK=-Lvendor/sdl2_windows/lib/x64/ -Lvendor/pthreads-win32/lib/ -lSDL2main -lSDL2
OUTPUT=bin/
NTHREADS?=16

all: main parallel cuda streams

main: $(OUTPUT)/main.exe

cuda: $(OUTPUT)/cuda.exe

streams: $(OUTPUT)/streams.exe

parallel: $(OUTPUT)/parallel.exe

$(OUTPUT)/main.exe: src/main.cpp bin/rays.obj
	$(GXX) $(FLAGS) -g -o $@ $^ $(INCLUDE) $(LINK) 

$(OUTPUT)/cuda.exe: src/main_cuda.cu bin/rays.obj
	$(GXX) $(FLAGS) -g -o $@ -DCUDA $^ $(INCLUDE) $(LINK) 

$(OUTPUT)/streams.exe: src/main_cuda.cu bin/rays.obj
	$(GXX) $(FLAGS) -g -o $@ -DSTREAMS $^ $(INCLUDE) $(LINK) 

$(OUTPUT)/parallel.exe: src/main.cpp bin/rays.obj bin/parallel_render.obj
	$(GXX) $(FLAGS)	-g -o $@ -DPARALLEL $^ $(INCLUDE) $(LINK) -lpthreadVC2 -DN_THREADS=$(NTHREADS)

bin/rays.obj: src/rays.cpp
	$(GXX) $(FLAGS) -g -c -o $@ $^ $(INCLUDE)

bin/parallel_render.obj: src/parallel_render.cpp
	$(GXX) $(FLAGS) -g -c -o $@ $^ $(INCLUDE)

clean:
	del bin\*.obj
	del bin\*.o
	del bin\*.exe
	del bin\*.pdb
	del bin\*.exp
	del bin\*.lib

