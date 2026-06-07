#!/usr/bin/env bash
# =====================================================================
#  Socium — pipeline completo en el clúster (build + correr + datos web)
#  Uso:   bash scripts/correr_cluster.sh [N_agentes] [hilos]
#  Ej.:   bash scripts/correr_cluster.sh 50000000 128
#  Los datos (data/reference/*.csv) ya vienen con el repo; no hay que bajarlos.
# =====================================================================
set -euo pipefail

N=${1:-50000000}
THREADS=${2:-$(nproc)}
PASO=$(( N / 80000 )); [ "$PASO" -lt 1 ] && PASO=1     # muestra ~80k agentes/escenario
cd "$(dirname "$0")/.."
export OMP_NUM_THREADS="$THREADS"

echo "== Socium cluster: N=$N  hilos=$THREADS  muestra-paso=$PASO =="

# 1) Compilar el núcleo (CMake si existe; si no, g++)
if command -v cmake >/dev/null 2>&1; then
  cmake -S core -B build -DCMAKE_BUILD_TYPE=Release -DSOCIUM_NATIVE=ON >/dev/null
  cmake --build build -j"$THREADS"
  BIN=./build/socium_simular
else
  echo "  (cmake no encontrado; compilo con g++)"
  g++ -std=c++20 -O3 -march=native -fopenmp -Icore/include \
      $(find core/src -name '*.cpp') core/apps/simulate.cpp -o /tmp/socium_simular
  BIN=/tmp/socium_simular
fi
echo "  binario: $BIN"

# 2) Datos: ya están en data/reference/ (vinieron con git clone).
#    Para refrescarlos desde las fuentes (requiere internet), descomenta:
# python3 scripts/fetch_divipola.py
# python3 scripts/fetch_poblacion_total.py 2026
# python3 scripts/fetch_poblacion_municipal.py 2024
# python3 scripts/fetch_conflicto.py 2022
# python3 scripts/fetch_pib_departamental.py 2023

mkdir -p experiments/runs

# 3) Generar palancas de cada escenario y correr la simulación a escala
for s in baseline cepeda de_la_espriella; do
  python3 scripts/build_scenario_params.py "$s" >/dev/null
  echo "-- corriendo escenario: $s --"
  EXTRA=""
  [ "$s" = baseline ] && EXTRA="--out-deptos experiments/runs/deptos_baseline.csv --out-distribucion experiments/runs/distribucion.csv"
  "$BIN" --n "$N" --years 4 --seed 7 --threads "$THREADS" \
     --escenario "data/reference/escenario_${s}.flat" \
     --out "experiments/runs/serie_${s}.csv" \
     --out-muestra "experiments/runs/muestra_${s}.csv" --muestra-paso "$PASO" \
     --out-perfiles "experiments/runs/perfiles_${s}.csv" $EXTRA
done

# 4) Empaquetar los datos para la web
python3 scripts/build_web_data.py

echo "== LISTO. Revisa web/data/ (perfiles.json, muestra.json, series.json, mapa.json). =="
echo "   Validar: python3 python/socium/viz/validar.py ; stylized_facts.py ; comparar_escenarios.py"
echo "   Publicar web: git add web && git commit -m 'datos 50M' && git push && git subtree push --prefix web origin gh-pages"
