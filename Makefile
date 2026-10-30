# =============================================================================
#  Mandelbrot en paralelo: secuencial, OpenMP y MPI
#
#    make          compila la versión secuencial en bin/
#    make clean    borra lo compilado y las imágenes generadas
# =============================================================================
CXX      = g++
CXXFLAGS = -O2 -Wall
BIN      = bin
COMUN    = src/mandelbrot_comun.h

all: $(BIN)/mandelbrot_secuencial

# Versión secuencial (referencia)
$(BIN)/mandelbrot_secuencial: src/mandelbrot_secuencial.cpp $(COMUN)
	@mkdir -p $(BIN)
	$(CXX) $(CXXFLAGS) $< -o $@

clean:
	rm -rf $(BIN) *.ppm

.PHONY: all clean
