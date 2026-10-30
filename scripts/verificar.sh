#!/bin/bash
# =============================================================================
#  verificar.sh - Comprueba que las versiones OpenMP y MPI calculen EXACTAMENTE
#                 lo mismo que la versión secuencial, con distintas cantidades
#                 de hilos/procesos y todos los tipos de reparto.
#
#  Compara dos cosas contra la versión secuencial:
#    1) la suma de control (total de iteraciones de todos los píxeles)
#    2) la imagen generada, byte por byte (comando cmp)
#
#  Uso:   make test        (o bien:  bash scripts/verificar.sh)
#  Sale con código 0 si todo coincide y con 1 si algo falla.
# =============================================================================
RAIZ="$(cd "$(dirname "$0")/.." && pwd)"
BIN="$RAIZ/bin"

for prog in mandelbrot_secuencial mandelbrot_omp mandelbrot_mpi; do
    if [ ! -x "$BIN/$prog" ]; then
        echo "Falta $BIN/$prog. Compilá primero con: make"
        exit 1
    fi
done

# Imagen chica para que la prueba sea rápida. El alto (480) es múltiplo de
# 1, 2, 3 y 4, así MPI no necesita ajustarlo.
PARAM="640 480 500"

# Si se piden más procesos MPI que núcleos, Open MPI necesita --oversubscribe
EXTRA=""
if mpirun --version 2>/dev/null | grep -q "Open MPI"; then EXTRA="--oversubscribe"; fi

TMP="$(mktemp -d)"
trap 'rm -rf "$TMP"' EXIT
cd "$TMP" || exit 1

suma() { grep "Suma de control" | grep -oE "[0-9]+$"; }

REF=$("$BIN/mandelbrot_secuencial" $PARAM | suma)
mv mandelbrot_secuencial.ppm referencia.ppm
echo "Referencia (secuencial): suma de control = $REF"
echo

FALLAS=0
comparar() {   # $1 = descripción, $2 = suma obtenida, $3 = imagen generada
    if [ "$2" = "$REF" ] && cmp -s "$3" referencia.ppm; then
        printf "  OK     %s\n" "$1"
    else
        printf "  FALLA  %s (suma %s)\n" "$1" "$2"
        FALLAS=$((FALLAS + 1))
    fi
}

echo "OpenMP:"
for modo in static dynamic guided; do
    for hilos in 1 2 4; do
        S=$("$BIN/mandelbrot_omp" "$hilos" "$modo" $PARAM | suma)
        comparar "$modo con $hilos hilo(s)" "$S" mandelbrot_omp.ppm
    done
done

echo "MPI:"
for modo in bloques ciclico; do
    for procesos in 1 2 3 4; do
        S=$(mpirun $EXTRA -np "$procesos" "$BIN/mandelbrot_mpi" "$modo" $PARAM | suma)
        comparar "$modo con $procesos proceso(s)" "$S" mandelbrot_mpi.ppm
    done
done

echo
if [ "$FALLAS" -eq 0 ]; then
    echo "Todo coincide: las tres versiones calculan exactamente lo mismo."
    exit 0
else
    echo "Hubo $FALLAS diferencias."
    exit 1
fi
