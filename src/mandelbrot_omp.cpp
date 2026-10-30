/*
 ============================================================================
 Nombre      : mandelbrot_omp.cpp
 Autor       : Francisco Martín Ruthlein
 Materia     : Paradigmas de Programación - FCEQyN, UNaM - 2026
 Descripción : Versión con OpenMP (MEMORIA COMPARTIDA, varios HILOS).
               Todos los hilos ven la misma imagen en memoria; cada uno
               calcula las filas que le asigna el planificador (schedule).

 Compilar :  make   (desde la carpeta principal; deja el ejecutable en bin/)
             o a mano, dentro de src/:
             g++ -O2 -fopenmp mandelbrot_omp.cpp -o mandelbrot_omp
 Ejecutar :  ./mandelbrot_omp [hilos] [static|dynamic|guided] [ancho] [alto] [max_iter]
     Ej.:    ./mandelbrot_omp 4 static
             ./mandelbrot_omp 4 dynamic
 ============================================================================
*/
#include <omp.h>
#include <iostream>
#include <string>
#include "mandelbrot_comun.h"
using namespace std;

// Datos globales: así el bucle paralelo queda corto y fácil de leer
int ancho, alto, max_iter;
vector<int> imagen;               // memoria COMPARTIDA por todos los hilos
vector<long long> trabajo_hilo;   // iteraciones que hizo cada hilo
vector<int> filas_hilo;           // filas que calculó cada hilo
vector<double> tiempo_hilo;       // tiempo que estuvo calculando cada hilo

// Calcula la fila y anota quién la hizo y cuánto le costó
long long procesar_fila(int y)
{
    double t = omp_get_wtime();
    long long it = calcular_fila(y, ancho, alto, max_iter, &imagen[(size_t)y * ancho]);
    int h = omp_get_thread_num();          // ¿qué hilo soy?
    trabajo_hilo[h] += it;                 // cada hilo escribe SOLO su casillero:
    filas_hilo[h]++;                       // no hay condición de carrera
    tiempo_hilo[h] += omp_get_wtime() - t;
    return it;
}

int main(int argc, char *argv[])
{
    int hilos = (argc > 1) ? atoi(argv[1]) : omp_get_max_threads();
    string modo = (argc > 2) ? argv[2] : "dynamic";
    leer_parametros(argc, argv, 3, ancho, alto, max_iter);
    if (hilos < 1) hilos = 1;
    if (modo != "static" && modo != "dynamic" && modo != "guided") {
        cout << "Modo desconocido '" << modo << "'. Use: static | dynamic | guided" << endl;
        return 1;
    }

    omp_set_num_threads(hilos);
    imagen.assign((size_t)ancho * alto, 0);
    trabajo_hilo.assign(hilos, 0);
    filas_hilo.assign(hilos, 0);
    tiempo_hilo.assign(hilos, 0.0);
    long long total = 0;                   // suma de control

    double inicio = omp_get_wtime();

    // ---------------- ZONA PARALELA ----------------
    // "parallel for": crea los hilos y reparte las vueltas del for entre ellos.
    // "reduction(+:total)": cada hilo suma en su copia privada y al final
    //                       OpenMP junta todas las copias en "total".
    // "schedule(...)": decide CÓMO se reparten las filas.
    if (modo == "static") {
        // static: bloques contiguos e iguales (hilo 0 las primeras filas, etc.)
        #pragma omp parallel for schedule(static) reduction(+:total)
        for (int y = 0; y < alto; y++)
            total += procesar_fila(y);
    } else if (modo == "dynamic") {
        // dynamic: cada hilo pide UNA fila; cuando termina, pide otra
        #pragma omp parallel for schedule(dynamic) reduction(+:total)
        for (int y = 0; y < alto; y++)
            total += procesar_fila(y);
    } else {
        // guided: como dynamic, pero los pedazos empiezan grandes y se achican
        #pragma omp parallel for schedule(guided) reduction(+:total)
        for (int y = 0; y < alto; y++)
            total += procesar_fila(y);
    }
    // ------------- FIN DE LA ZONA PARALELA ----------

    double segundos = omp_get_wtime() - inicio;

    cout << "=== Mandelbrot OpenMP (" << hilos << " hilos, schedule " << modo << ") ===" << endl;
    cout << "Imagen: " << ancho << " x " << alto << " pixeles, max_iter = " << max_iter << endl;
    cout << "Tiempo de calculo: " << segundos << " s" << endl;
    cout << "Suma de control (iteraciones totales): " << total << endl;
    cout << endl << "Hilo | Filas | % del trabajo | Tiempo calculando (s)" << endl;
    double max_pct = 0;
    for (int h = 0; h < hilos; h++) {
        double pct = 100.0 * trabajo_hilo[h] / total;
        if (pct > max_pct) max_pct = pct;
        printf("%4d | %5d | %12.1f%% | %8.3f\n", h, filas_hilo[h], pct, tiempo_hilo[h]);
    }
    printf("Hilo mas cargado: %.1f%% del trabajo (lo ideal seria %.1f%%)\n", max_pct, 100.0 / hilos);

    if (guardar_ppm("mandelbrot_omp.ppm", imagen, ancho, alto, max_iter))
        cout << "Imagen guardada en mandelbrot_omp.ppm" << endl;
    return 0;
}
