# Toy implementation of a ray-casting algorithm written in C using SDL2.
Sebastián Aedo
201573057-1

# Instrucciones de compilación
Se require de la librería SDL para compilar en Linux (disponible
en la gran mayoría de los repositorios por defecto), en el caso 
de Ubuntu:

```
sudo apt install libsdl2-dev
```

# Lineal
Para la forma lineal, simplemente ejecutar `make`

# Paralela
Para la forma paralela, se puede cambiar en el `Makefile` el segundo argumento
que corresponde a la cantidad de threads con la que se compilará.

# Ejecución

Para la ejecución, es:
```
./main 2 # para la versión lineal
./parallel 2 # para la versión paralela
```
donde 2 corresponde a la cantidad de esferas que se renderizarán.

Los números que saldrán en la consola corresponden a los Frames Per Second
