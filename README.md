# Toy implementation of a ray-casting algorithm written in C using SDL2.

Para compilar:

```
# Compila todos los targets
make

# Compila la version cuda naive
make cuda

# Compila la version que usa streams
make streams

# Compila la version CPU paralela
make parallel

# Compila la version secuencial
make main
```

Para ejecutar, dependiendo de la que se desea: `.\bin\<version> <n_esferas>`. 
Por ejemplo:
```
# Ejecuta la version cuda naive con 10 esferas
.\bin\cuda.exe 10

# Ejecuta la version que usa streams con 10 esferas
.\bin\streams.exe 10

# Ejecuta la version CPU paralela con 10 esferas
.\bin\parallel.exe 10

# Ejecuta la version secuencial con 10 esferas
.\bin\main.exe 10
```
