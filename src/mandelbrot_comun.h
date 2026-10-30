/*
 ============================================================================
 Nombre      : mandelbrot_comun.h
 Autor       : Francisco Martín Ruthlein
 Materia     : Paradigmas de Programación - FCEQyN, UNaM - 2026
 Descripción : Código que comparten las tres versiones (secuencial, OpenMP y
               MPI). El cálculo es EXACTAMENTE el mismo en las tres: lo único
               que cambia entre versiones es CÓMO se reparten las filas.
 ============================================================================

 ¿Qué es el conjunto de Mandelbrot?
   Cada píxel de la imagen representa un número complejo c = (cx, cy).
   Arrancamos con z = 0 y repetimos  z = z*z + c.
   - Si |z| se mantiene chico (|z| <= 2) durante "max_iter" vueltas,
     decimos que c PERTENECE al conjunto -> se pinta de negro.
   - Si |z| se pasa de 2, ya se sabe que se va al infinito: cortamos y
     guardamos en qué vuelta se escapó -> eso define el color.

   Detalle clave para la paralelización:
   los píxeles NEGROS son los más caros (hacen las max_iter vueltas
   completas) y los de afuera se resuelven en pocas vueltas. Por eso
   unas filas cuestan muchísimo más que otras.
*/

#ifndef MANDELBROT_COMUN_H
#define MANDELBROT_COMUN_H

#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <vector>

// Zona del plano complejo que se dibuja (la vista clásica completa)
const double X_MIN = -2.5, X_MAX = 1.0;
const double Y_MIN = -1.25, Y_MAX = 1.25;

// Parámetros por defecto (se pueden cambiar por línea de comandos)
const int ANCHO_DEF = 2016;
const int ALTO_DEF = 1440;
const int ITER_DEF = 2000;

// Cantidad de vueltas que hace un píxel antes de escapar (máximo max_iter)
inline int iteraciones_pixel(double cx, double cy, int max_iter)
{
    double zx = 0.0, zy = 0.0;
    int i = 0;
    while (i < max_iter && zx * zx + zy * zy <= 4.0) {   // |z|^2 <= 4  <=>  |z| <= 2
        double tmp = zx * zx - zy * zy + cx;              // parte real de z*z + c
        zy = 2.0 * zx * zy + cy;                          // parte imaginaria de z*z + c
        zx = tmp;
        i++;
    }
    return i;
}

/*
 Calcula UNA fila completa de la imagen y guarda el resultado en "fila".
 Devuelve cuántas iteraciones hizo en total esa fila (sirve para medir
 cuánto "trabajo" costó y como suma de control).
 Cada fila es independiente de las demás: por eso se puede repartir.
*/
inline long long calcular_fila(int y, int ancho, int alto, int max_iter, int *fila)
{
    double cy = Y_MAX - y * (Y_MAX - Y_MIN) / (alto - 1);
    long long trabajo = 0;
    for (int x = 0; x < ancho; x++) {
        double cx = X_MIN + x * (X_MAX - X_MIN) / (ancho - 1);
        int it = iteraciones_pixel(cx, cy, max_iter);
        fila[x] = it;
        trabajo += it;
    }
    return trabajo;
}

// Lee ancho, alto y max_iter de los argumentos (a partir de la posición "desde")
inline void leer_parametros(int argc, char *argv[], int desde, int &ancho, int &alto, int &max_iter)
{
    ancho = (argc > desde) ? atoi(argv[desde]) : ANCHO_DEF;
    alto = (argc > desde + 1) ? atoi(argv[desde + 1]) : ALTO_DEF;
    max_iter = (argc > desde + 2) ? atoi(argv[desde + 2]) : ITER_DEF;
    if (ancho < 2) ancho = ANCHO_DEF;
    if (alto < 2) alto = ALTO_DEF;
    if (max_iter < 1) max_iter = ITER_DEF;
}

/*
 Guarda la imagen en formato PPM (P6): es el formato de imagen más simple
 que existe (un encabezado de texto + los bytes RGB de cada píxel).
 Se abre con el visor de imágenes de Ubuntu, GIMP, o se convierte a PNG con:
     convert mandelbrot.ppm mandelbrot.png
*/
inline bool guardar_ppm(const char *nombre, const std::vector<int> &img, int ancho, int alto, int max_iter)
{
    FILE *f = fopen(nombre, "wb");
    if (!f) return false;
    fprintf(f, "P6\n%d %d\n255\n", ancho, alto);
    std::vector<unsigned char> rgb(3 * (size_t)ancho);
    for (int y = 0; y < alto; y++) {
        for (int x = 0; x < ancho; x++) {
            int it = img[(size_t)y * ancho + x];
            unsigned char r = 0, g = 0, b = 0;            // dentro del conjunto: negro
            if (it < max_iter) {
                // Paleta suave: t entre 0 y 1 según qué tan rápido escapó (escala logarítmica)
                double t = log(1.0 + it) / log(1.0 + max_iter);
                r = (unsigned char)(255 * 9.0 * (1 - t) * t * t * t);
                g = (unsigned char)(255 * 15.0 * (1 - t) * (1 - t) * t * t);
                b = (unsigned char)(255 * 8.5 * (1 - t) * (1 - t) * (1 - t) * t);
            }
            rgb[3 * x] = r;
            rgb[3 * x + 1] = g;
            rgb[3 * x + 2] = b;
        }
        fwrite(rgb.data(), 1, rgb.size(), f);
    }
    fclose(f);
    return true;
}

#endif
