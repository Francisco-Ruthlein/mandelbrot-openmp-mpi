/*
 ============================================================================
 Nombre      : mandelbrot_secuencial.cpp
 Autor       : Francisco Martín Ruthlein
 Materia     : Paradigmas de Programación - FCEQyN, UNaM - 2026
 Descripción : Versión SECUENCIAL (un solo hilo, un solo proceso).
               Es la referencia: contra esta versión comparamos el tiempo
               y verificamos que las versiones paralelas den lo mismo.

 Compilar :  make   (desde la carpeta principal; deja el ejecutable en bin/)
             o a mano, dentro de src/:
             g++ -O2 mandelbrot_secuencial.cpp -o mandelbrot_secuencial
 Ejecutar :  ./mandelbrot_secuencial [ancho] [alto] [max_iter]
             ./mandelbrot_secuencial            (usa 2016 x 1440, 2000 iter)
 ============================================================================
*/
#include "mandelbrot_comun.h"
#include <iostream>
#include <chrono>
using namespace std;

int main(int argc, char *argv[])
{
    int ancho, alto, max_iter;
    leer_parametros(argc, argv, 1, ancho, alto, max_iter);

    vector<int> imagen((size_t)ancho * alto);   // iteraciones de cada píxel
    long long total = 0;                        // suma de control

    auto inicio = chrono::steady_clock::now();

    // Recorremos las filas una por una, en orden
    for (int y = 0; y < alto; y++)
        total += calcular_fila(y, ancho, alto, max_iter, &imagen[(size_t)y * ancho]);

    auto fin = chrono::steady_clock::now();
    double segundos = chrono::duration<double>(fin - inicio).count();

    cout << "=== Mandelbrot SECUENCIAL ===" << endl;
    cout << "Imagen: " << ancho << " x " << alto << " pixeles, max_iter = " << max_iter << endl;
    cout << "Tiempo de calculo: " << segundos << " s" << endl;
    cout << "Suma de control (iteraciones totales): " << total << endl;

    if (guardar_ppm("mandelbrot_secuencial.ppm", imagen, ancho, alto, max_iter))
        cout << "Imagen guardada en mandelbrot_secuencial.ppm" << endl;
    return 0;
}
