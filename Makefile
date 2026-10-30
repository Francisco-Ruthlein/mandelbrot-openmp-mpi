# =============================================================================
#  Mandelbrot en paralelo: secuencial, OpenMP y MPI
#
#    make          compila las tres versiones en bin/
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

clean:
	rm -rf $(BIN) *.ppm

.PHONY: all clean
