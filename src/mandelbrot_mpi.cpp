/*
 ============================================================================
 Nombre      : mandelbrot_mpi.cpp
 Autor       : Francisco Martín Ruthlein
 Materia     : Paradigmas de Programación - FCEQyN, UNaM - 2026
 Descripción : Versión con MPI (MEMORIA DISTRIBUIDA, varios PROCESOS).
               Cada proceso tiene su propia memoria: calcula SUS filas en un
               arreglo local y después se las MANDA al proceso 0, que arma
               la imagen completa. Se comunican solo por mensajes.

 Compilar :  make   (desde la carpeta principal; deja el ejecutable en bin/)
             o a mano, dentro de src/:
             mpicxx -O2 mandelbrot_mpi.cpp -o mandelbrot_mpi
 Ejecutar :  mpirun -np 4 ./mandelbrot_mpi [bloques|ciclico] [ancho] [alto] [max_iter]
     Ej.:    mpirun -np 4 ./mandelbrot_mpi bloques
             mpirun -np 4 ./mandelbrot_mpi ciclico
 ============================================================================

 Dos formas de repartir las filas entre P procesos:
   bloques : el proceso 0 hace el primer pedazo de filas seguidas, el 1 el
             segundo pedazo, etc.        (0 0 0 1 1 1 2 2 2 3 3 3)
   ciclico : las filas se reparten como cartas: fila 0 al proceso 0, fila 1
             al proceso 1, ... y se vuelve a empezar.   (0 1 2 3 0 1 2 3 ...)
*/
#include "mpi.h"
#include <iostream>
#include <string>
#include "mandelbrot_comun.h"
using namespace std;

const int BLOQUES = 0, CICLICO = 1;

// Devuelve el número de la k-ésima fila que le toca al proceso p
int fila_global(int k, int p, int procesos, int filas_por_proceso, int modo)
{
    if (modo == BLOQUES) return p * filas_por_proceso + k;   // filas seguidas
    else                 return k * procesos + p;            // repartidas como cartas
}

int main(int argc, char *argv[])
{
    int rank, size;
    MPI_Init(&argc, &argv);                       // arranca el entorno MPI
    MPI_Comm_size(MPI_COMM_WORLD, &size);         // ¿cuántos procesos somos?
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);         // ¿qué número de proceso soy?

    // ---- 1) El proceso 0 lee los parámetros y se los DIFUNDE a todos ----
    int param[4];                                 // ancho, alto, max_iter, modo
    if (rank == 0) {
        string modo_txt = (argc > 1) ? argv[1] : "ciclico";
        leer_parametros(argc, argv, 2, param[0], param[1], param[2]);
        param[3] = (modo_txt == "bloques") ? BLOQUES : CICLICO;
        if (modo_txt != "bloques" && modo_txt != "ciclico")
            cout << "Modo desconocido '" << modo_txt << "', uso ciclico" << endl;
        // Igual que en el laboratorio de producto escalar: el alto tiene que
        // ser múltiplo de la cantidad de procesos para repartir parejo
        if (param[1] % size != 0) {
            param[1] = (param[1] / size + 1) * size;
            cout << "Alto ajustado a " << param[1] << " (multiplo de " << size << ")" << endl;
        }
    }
    MPI_Bcast(param, 4, MPI_INT, 0, MPI_COMM_WORLD);
    int ancho = param[0], alto = param[1], max_iter = param[2], modo = param[3];
    int filas_por_proceso = alto / size;

    // ---- 2) Cada proceso calcula SUS filas en SU memoria local ----
    vector<int> local((size_t)filas_por_proceso * ancho);
    long long mi_trabajo = 0;

    MPI_Barrier(MPI_COMM_WORLD);                  // todos arrancan juntos
    double inicio = MPI_Wtime();

    for (int k = 0; k < filas_por_proceso; k++) {
        int y = fila_global(k, rank, size, filas_por_proceso, modo);
        mi_trabajo += calcular_fila(y, ancho, alto, max_iter, &local[(size_t)k * ancho]);
    }
    double mi_tiempo = MPI_Wtime() - inicio;

    // ---- 3) Comunicación: el proceso 0 junta (Gather) los pedazos ----
    vector<int> recibido;
    if (rank == 0) recibido.resize((size_t)alto * ancho);
    MPI_Gather(local.data(), filas_por_proceso * ancho, MPI_INT,
               recibido.data(), filas_por_proceso * ancho, MPI_INT,
               0, MPI_COMM_WORLD);

    // También juntamos cuánto trabajó y cuánto tardó cada uno (para el informe)
    vector<long long> trabajos(size);
    vector<double> tiempos(size);
    MPI_Gather(&mi_trabajo, 1, MPI_LONG_LONG, trabajos.data(), 1, MPI_LONG_LONG, 0, MPI_COMM_WORLD);
    MPI_Gather(&mi_tiempo, 1, MPI_DOUBLE, tiempos.data(), 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    // Suma de control: Reduce suma el trabajo de todos en el proceso 0
    long long total = 0;
    MPI_Reduce(&mi_trabajo, &total, 1, MPI_LONG_LONG, MPI_SUM, 0, MPI_COMM_WORLD);

    double tiempo_total = MPI_Wtime() - inicio;   // cálculo + comunicación

    // ---- 4) El proceso 0 ordena las filas y muestra resultados ----
    if (rank == 0) {
        // Gather deja los datos ordenados por proceso (primero todo lo del 0,
        // después todo lo del 1...). Los ponemos en su fila real de la imagen.
        vector<int> imagen((size_t)alto * ancho);
        for (int p = 0; p < size; p++)
            for (int k = 0; k < filas_por_proceso; k++) {
                int y = fila_global(k, p, size, filas_por_proceso, modo);
                const int *origen = &recibido[((size_t)p * filas_por_proceso + k) * ancho];
                copy(origen, origen + ancho, &imagen[(size_t)y * ancho]);
            }

        cout << "=== Mandelbrot MPI (" << size << " procesos, reparto "
             << (modo == BLOQUES ? "bloques" : "ciclico") << ") ===" << endl;
        cout << "Imagen: " << ancho << " x " << alto << " pixeles, max_iter = " << max_iter << endl;
        cout << "Tiempo total (calculo + comunicacion): " << tiempo_total << " s" << endl;
        cout << "Suma de control (iteraciones totales): " << total << endl;
        cout << endl << "Proceso | Filas | % del trabajo | Tiempo calculando (s)" << endl;
        double max_pct = 0;
        for (int p = 0; p < size; p++) {
            double pct = 100.0 * trabajos[p] / total;
            if (pct > max_pct) max_pct = pct;
            printf("%7d | %5d | %12.1f%% | %8.3f\n", p, filas_por_proceso, pct, tiempos[p]);
        }
        printf("Proceso mas cargado: %.1f%% del trabajo (lo ideal seria %.1f%%)\n", max_pct, 100.0 / size);

        if (guardar_ppm("mandelbrot_mpi.ppm", imagen, ancho, alto, max_iter))
            cout << "Imagen guardada en mandelbrot_mpi.ppm" << endl;
    }

    MPI_Finalize();                               // cierra el entorno MPI
    return 0;
}
