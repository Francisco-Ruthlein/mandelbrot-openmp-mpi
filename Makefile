# =============================================================================
#  Mandelbrot en paralelo: secuencial, OpenMP y MPI
#
#    make          compila las tres versiones en bin/
#    make test     verifica que las tres den exactamente el mismo resultado
#    make bench    mide tiempos y speedup con la imagen completa
#    make clean    borra lo compilado y las imágenes generadas
# =============================================================================
CXX      = g++
MPICXX   = mpicxx
CXXFLAGS = -O2 -Wall
BIN      = bin
COMUN    = src/mandelbrot_comun.h

all: $(BIN)/mandelbrot_secuencial $(BIN)/mandelbrot_omp $(BIN)/mandelbrot_mpi

# Versión secuencial (referencia)
$(BIN)/mandelbrot_secuencial: src/mandelbrot_secuencial.cpp $(COMUN)
	@mkdir -p $(BIN)
	$(CXX) $(CXXFLAGS) $< -o $@

# Versión OpenMP: hilos con memoria compartida (-fopenmp activa los #pragma omp)
$(BIN)/mandelbrot_omp: src/mandelbrot_omp.cpp $(COMUN)
	@mkdir -p $(BIN)
	$(CXX) $(CXXFLAGS) -fopenmp $< -o $@

# Versión MPI: procesos con memoria distribuida (mpicxx = g++ + biblioteca MPI)
$(BIN)/mandelbrot_mpi: src/mandelbrot_mpi.cpp $(COMUN)
	@mkdir -p $(BIN)
	$(MPICXX) $(CXXFLAGS) $< -o $@

test: all
	bash scripts/verificar.sh

bench: all
	bash scripts/medir.sh

clean:
	rm -rf $(BIN) *.ppm

.PHONY: all test bench clean
