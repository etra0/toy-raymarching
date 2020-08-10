# Toy implementation of a ray-casting algorithm written in C using SDL2.

Para compilar:

```
# Version secuencial
make -f Makefile.windows main

# Version CUDA
make -f Makefile.windows cuda

# Version CPU paralela (Requiere minGW)
make GXX=g++.exe FLAGS=-O2 -f Makefile.windows parallel
```
