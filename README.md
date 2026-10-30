# Mandelbrot en paralelo: OpenMP y MPI

Trabajo práctico de **Paradigmas de Programación** (Analista de Sistemas de Computación, FCEQyN – UNaM, 2026).

Objetivo: dibujar el conjunto de Mandelbrot de tres formas (secuencial, con OpenMP y con MPI), comprobar que las tres den exactamente el mismo resultado y medir cuánto se acelera.

- **OpenMP:** hilos con memoria compartida.
- **MPI:** procesos con memoria distribuida.

## Avance

- [x] Estructura inicial del proyecto
- [x] Cálculo común del fractal (`src/mandelbrot_comun.h`)
- [x] Versión secuencial de referencia
- [x] Versión OpenMP (hilos, memoria compartida)
- [x] Versión MPI (procesos, memoria distribuida)
- [x] Verificación automática (`make test`)
- [ ] Medición de tiempos (`make bench`)
- [ ] Integración continua (GitHub Actions) y Codespaces
- [ ] Documentación completa

Autor: Francisco Martín Ruthlein
