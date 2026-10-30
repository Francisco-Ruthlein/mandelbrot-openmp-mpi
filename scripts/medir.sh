#!/bin/bash
# =============================================================================
#  medir.sh - Ejecuta las tres versiones con la imagen completa (2016 x 1440,
#             2000 iteraciones) y arma una tabla de tiempos y speedup.
#
#  Uso:   make bench                 (o bien:  bash scripts/medir.sh [repeticiones])
#  Repite cada prueba 3 veces (por defecto) y se queda con el mejor tiempo.
# =============================================================================
REP=${1:-3}
RAIZ="$(cd "$(dirname "$0")/.." && pwd)"
BIN="$RAIZ/bin"
NUCLEOS=$(nproc)

for prog in mandelbrot_secuencial mandelbrot_omp mandelbrot_mpi; do
    if [ ! -x "$BIN/$prog" ]; then
        echo "Falta $BIN/$prog. Compilá primero con: make"
        exit 1
    fi
done

# Si se piden más procesos MPI que núcleos, Open MPI necesita --oversubscribe
EXTRA=""
if mpirun --version 2>/dev/null | grep -q "Open MPI"; then EXTRA="--oversubscribe"; fi

# Cantidades de hilos/procesos a probar: 1, 2, 4 y la cantidad de núcleos
LISTA="1 2 4"
case " $LISTA " in *" $NUCLEOS "*) ;; *) LISTA="$LISTA $NUCLEOS" ;; esac

# Las imágenes que generan los programas van a una carpeta temporal
TMP="$(mktemp -d)"
trap 'rm -rf "$TMP"' EXIT
cd "$TMP" || exit 1

# Ejecuta un comando REP veces y devuelve: mejor_tiempo  suma  %_del_más_cargado
correr() {
    local mejor="" suma="" pct="-"
    for ((i = 1; i <= REP; i++)); do
        local salida t p
        salida=$("$@")
        t=$(echo "$salida" | grep -E "^Tiempo" | head -1 | grep -oE "[0-9]+\.[0-9]+")
        suma=$(echo "$salida" | grep "Suma de control" | grep -oE "[0-9]+$")
        p=$(echo "$salida" | grep -E "mas cargado" | grep -oE "[0-9]+\.[0-9]+%" | head -1)
        [ -n "$p" ] && pct=$p
        if [ -z "$mejor" ] || awk "BEGIN{exit !($t < $mejor)}"; then mejor=$t; fi
    done
    echo "$mejor $suma $pct"
}

fila() { printf "%-11s %-9s %-13s %-11s %-8s %-14s %s\n" "$@"; }

echo "Maquina: $NUCLEOS nucleos | repeticiones por prueba: $REP"
echo
fila "Version" "Reparto" "Hilos/Procs" "Tiempo(s)" "Speedup" "Mas cargado" "Suma de control"
printf -- "-----------------------------------------------------------------------------------------\n"

read -r TSEQ SUMA _ <<< "$(correr "$BIN/mandelbrot_secuencial")"
fila "Secuencial" "-" "1" "$TSEQ" "1.00" "-" "$SUMA"

for modo in static dynamic; do
    for n in $LISTA; do
        read -r T S P <<< "$(correr "$BIN/mandelbrot_omp" "$n" "$modo")"
        fila "OpenMP" "$modo" "$n" "$T" "$(awk "BEGIN{printf \"%.2f\", $TSEQ/$T}")" "$P" "$S"
    done
done

for modo in bloques ciclico; do
    for n in $LISTA; do
        read -r T S P <<< "$(correr mpirun $EXTRA -np "$n" "$BIN/mandelbrot_mpi" "$modo")"
        fila "MPI" "$modo" "$n" "$T" "$(awk "BEGIN{printf \"%.2f\", $TSEQ/$T}")" "$P" "$S"
    done
done

echo
echo "Si todas las sumas de control son iguales, las tres versiones calcularon lo mismo."
echo "Speedup = tiempo secuencial / tiempo paralelo. Con N nucleos, lo ideal es N."
echo "'Mas cargado' = porcentaje del trabajo total que hizo el hilo o proceso mas cargado."
