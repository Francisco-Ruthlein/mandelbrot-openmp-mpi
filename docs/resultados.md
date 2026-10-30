# Resultados

## Máquina de prueba

| | |
|---|---|
| Sistema | Linux (Ubuntu 24.04) |
| Núcleos | 2 |
| Compilador | g++ 13.3, con `-O2` |
| MPI | Open MPI 4.1.6 |
| Imagen | 2016 × 1440 píxeles, hasta 2000 iteraciones por píxel |
| Método | `make bench`: cada prueba se corre 3 veces y se toma el mejor tiempo |

## Tiempos

| Versión | Reparto | Hilos / procesos | Tiempo | Speedup | El más cargado hizo |
|---|---|---|---|---|---|
| Secuencial | — | 1 | 2,29 s | 1,00 | — |
| OpenMP | static | 2 | 1,16 s | 1,98 | 50,0 % |
| OpenMP | dynamic | 2 | 1,15 s | 2,00 | 50,3 % |
| MPI | bloques | 2 | 1,13 s | 2,02 | 50,0 % |
| MPI | cíclico | 2 | 1,13 s | 2,03 | 50,0 % |
| OpenMP | static | 4 | 1,16 s | 1,97 | **47,2 %** |
| OpenMP | dynamic | 4 | 1,15 s | 2,00 | 28,0 % |
| MPI | bloques | 4 | 1,14 s | 2,01 | **47,2 %** |
| MPI | cíclico | 4 | 1,13 s | 2,03 | **25,0 %** |

- **Speedup** = tiempo secuencial ÷ tiempo paralelo. Con N núcleos, lo ideal es N.
- **El más cargado hizo** = el porcentaje del trabajo total (iteraciones) que le tocó al hilo o proceso con más trabajo. Lo ideal es 100 % ÷ N.
- La **suma de control** fue 1.014.493.575 iteraciones en todas las pruebas. Las imágenes son idénticas byte a byte a la secuencial (`make test`).

## Cómo leerlos

**Con 2 núcleos, todas las versiones llegan al máximo posible (≈ 2).** Con 2 trabajadores, incluso `static` y `bloques` quedan balanceados: la imagen es simétrica arriba y abajo, así que cada mitad tiene justo el 50 % del trabajo.

**Con 4 trabajadores, lo que cambia es el reparto.** Las filas del medio de la imagen atraviesan la zona negra y cuestan hasta unas 480 veces más que las de los bordes. Repartiendo en bloques iguales de filas (`static` en OpenMP, `bloques` en MPI), dos de los cuatro trabajadores se quedan con el 94 % del trabajo. Con `dynamic` o `ciclico`, cada uno hace ≈ 25 %.

**Por qué con 4 no baja el tiempo en esta máquina.** Hay 2 núcleos para 4 trabajadores, así que el sistema operativo los turna. Cuando los dos livianos terminan, los dos pesados quedan uno en cada núcleo y el tiempo total no cambia.

**Qué pasaría con 4 núcleos reales.** Ningún programa puede terminar antes que su trabajador más cargado. Con `static` o `bloques`, ese trabajador hace el 47,2 % del total, así que el speedup no puede superar 100 ÷ 47,2 ≈ **2,1**. Con `dynamic` o `ciclico` (≈ 25 % cada uno) el techo es ≈ **4**.

**Prueba de que el efecto se ve en el reloj.** En esta misma máquina, con 3 hilos:

| Schedule | Tiempo (mejor de 3) | Hilo más cargado |
|---|---|---|
| static | 1,79 s | 73,1 % |
| dynamic | 1,15 s | entre 34 % y 37 % (ideal: 33,3 %) |

Con `static` el hilo del medio carga el 73 % del trabajo, y el programa tarda lo que tarda ese hilo.

**Otros detalles.** El 28,0 % de `dynamic` con 4 hilos varía en cada corrida (entre 25 % y 29 %), porque depende de cómo el sistema operativo turna los hilos. El speedup de 2,03 en MPI, apenas arriba de 2, es ruido de medición.

## Reproducirlo

```bash
make bench           # o: bash scripts/medir.sh 5   (5 repeticiones)
```

En una PC con 4 núcleos o más, la diferencia entre `static` y `dynamic` con 4 hilos se ve directamente en los tiempos.
