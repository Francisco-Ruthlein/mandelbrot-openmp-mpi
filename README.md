# Mandelbrot en paralelo: OpenMP y MPI

[![Compilar y verificar](https://github.com/Francisco-Ruthlein/mandelbrot-openmp-mpi/actions/workflows/compilar-y-verificar.yml/badge.svg)](https://github.com/Francisco-Ruthlein/mandelbrot-openmp-mpi/actions/workflows/compilar-y-verificar.yml)

El fractal de Mandelbrot calculado en C++ de tres formas: **secuencial**, con **OpenMP** (hilos con memoria compartida) y con **MPI** (procesos con memoria distribuida). El proyecto verifica automáticamente que las tres versiones den exactamente el mismo resultado y mide el speedup.

![Conjunto de Mandelbrot generado por el programa](docs/img/mandelbrot.png)

> **English summary.** A C++ Mandelbrot renderer in three versions: sequential, OpenMP (shared memory) and MPI (distributed memory). All three produce byte-identical images, which CI checks on every push. The project shows why splitting rows into equal blocks balances this problem poorly, since central rows cost up to ~480× more than edge rows. It also shows how `schedule(dynamic)` in OpenMP and a cyclic row distribution in MPI fix the load imbalance.

## Lo más interesante: el balanceo de carga

Cada fila se calcula sin depender de las demás, pero no todas cuestan lo mismo: las del medio atraviesan la zona negra y cuestan hasta unas 480 veces más que las de los bordes. Si se reparten bloques iguales de filas, dos de cuatro trabajadores terminan con el 94 % del trabajo.

![Reparto del trabajo con 4 trabajadores](docs/img/reparto_trabajo.png)

La solución en OpenMP es `schedule(dynamic)`: cada hilo pide una fila cuando termina la anterior. En MPI es el reparto **cíclico**: las filas se reparten como cartas. Con cualquiera de las dos, cada trabajador hace ≈ 25 %. La explicación completa está en [docs/explicacion.md](docs/explicacion.md).

## Resultados

Medidos en una máquina Linux de 2 núcleos, con la imagen de 2016 × 1440 píxeles y 2000 iteraciones:

| Versión | Tiempo | Speedup |
|---|---|---|
| Secuencial | 2,29 s | 1,0 |
| OpenMP, 2 hilos (`dynamic`) | 1,15 s | 2,0 |
| MPI, 2 procesos (`ciclico`) | 1,13 s | 2,0 |

Con 2 núcleos, 2,0 es el máximo posible. La tabla completa, la explicación del límite con 4 núcleos y cómo reproducir las mediciones están en [docs/resultados.md](docs/resultados.md).

## Probarlo

### En tu computadora (Linux o WSL en Windows)

```bash
sudo apt-get install g++ make openmpi-bin libopenmpi-dev
git clone https://github.com/Francisco-Ruthlein/mandelbrot-openmp-mpi.git
cd mandelbrot-openmp-mpi
make          # compila las tres versiones en bin/
make test     # verifica que las tres den el mismo resultado
```

### En el navegador, sin instalar nada (GitHub Codespaces)

En la página del repositorio: **Code → Codespaces → Create codespace on main**. El entorno instala Open MPI y compila solo. Cuando termina, en la terminal: `make test`.

## Ejecutar

```bash
./bin/mandelbrot_secuencial

# OpenMP:  ./bin/mandelbrot_omp <hilos> <static|dynamic|guided>
./bin/mandelbrot_omp 4 static
./bin/mandelbrot_omp 4 dynamic

# MPI:  mpirun -np <procesos> ./bin/mandelbrot_mpi <bloques|ciclico>
mpirun -np 4 ./bin/mandelbrot_mpi bloques
mpirun -np 4 ./bin/mandelbrot_mpi ciclico

# Todas las pruebas, con tabla de tiempos y speedup
make bench
```

Cada ejecución muestra:

- el tiempo de cálculo;
- la suma de control (el total de iteraciones);
- una tabla con las filas, el % del trabajo y el tiempo de cada hilo o proceso.

También guarda la imagen en un archivo `.ppm`, que se abre con el visor de imágenes de Ubuntu o se convierte con `convert archivo.ppm imagen.png`.

**Notas:**

- Al final de cada comando se puede agregar `[ancho] [alto] [max_iter]`, por ejemplo `./bin/mandelbrot_omp 4 dynamic 1000 720 500`.
- Si la PC tiene menos núcleos que los procesos pedidos, Open MPI pide agregar `--oversubscribe`: `mpirun --oversubscribe -np 4 ...`.
- También se puede compilar a mano, igual que en los laboratorios de la materia, desde `src/`: `g++ -O2 -fopenmp mandelbrot_omp.cpp -o mandelbrot_omp`.

## Estructura

```
├── src/
│   ├── mandelbrot_comun.h          cálculo compartido por las tres versiones
│   ├── mandelbrot_secuencial.cpp   versión de referencia
│   ├── mandelbrot_omp.cpp          OpenMP: hilos, schedule static / dynamic / guided
│   └── mandelbrot_mpi.cpp          MPI: procesos, reparto por bloques / cíclico
├── scripts/
│   ├── verificar.sh                compara sumas de control e imágenes (make test)
│   └── medir.sh                    tabla de tiempos y speedup (make bench)
├── docs/
│   ├── explicacion.md              cómo funciona, paso a paso
│   ├── resultados.md               mediciones y cómo leerlas
│   └── img/
├── .github/workflows/              compila y verifica en cada push
├── .devcontainer/                  entorno listo para GitHub Codespaces
└── Makefile
```

## Qué se usó

- **C++** con g++ y `-O2`.
- **OpenMP:**
  - `#pragma omp parallel for`
  - `schedule(static | dynamic | guided)`
  - `reduction(+:total)`
  - `omp_get_thread_num` y `omp_get_wtime`
- **MPI (Open MPI):**
  - `MPI_Init` / `MPI_Finalize`, `MPI_Comm_rank` / `MPI_Comm_size`
  - `MPI_Bcast`, `MPI_Barrier`, `MPI_Gather` y `MPI_Reduce`
  - `MPI_Wtime`
- **GitHub Actions** compila y corre la verificación en cada cambio. **Dev Containers** prepara el entorno de Codespaces.

## Contexto

Trabajo práctico de **Paradigmas de Programación**, de la carrera Analista de Sistemas de Computación (FCEQyN, Universidad Nacional de Misiones, 2026). Está basado en los laboratorios de MPI y OpenMP de la cátedra; las referencias están en [docs/explicacion.md](docs/explicacion.md#referencias).

## Autor

Francisco Martín Ruthlein

## Licencia

MIT. Ver [LICENSE](LICENSE).
