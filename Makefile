# =============================================================================
#  Mandelbrot en paralelo: secuencial, OpenMP y MPI
#
#    make          compila las versiones secuencial y OpenMP en bin/
#    make clean    borra lo compilado y las imágenes generadas
# =============================================================================
CXX      = g++
CXXFLAGS = -O2 -Wall
BIN      = bin
COMUN    = src/mandelbrot_comun.h

all: $(BIN)/mandelbrot_secuencial $(BIN)/mandelbrot_omp

# Versión secuencial (referencia)
$(BIN)/mandelbrot_secuencial: src/mandelbrot_secuencial.cpp $(COMUN)
	@mkdir -p $(BIN)
	$(CXX) $(CXXFLAGS) $< -o $@

# Versión OpenMP: hilos con memoria compartida (-fopenmp activa los #pragma omp)
$(BIN)/mandelbrot_omp: src/mandelbrot_omp.cpp $(COMUN)
	@mkdir -p $(BIN)
	$(CXX) $(CXXFLAGS) -fopenmp $< -o $@

clean:
	rm -rf $(BIN) *.ppm

.PHONY: all clean
