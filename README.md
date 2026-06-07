# Socium

**Gemelo digital de Colombia para comparar escenarios de política pública.**

Socium es un proyecto de **sociofísica + HPC** que construye un modelo basado en
agentes (ABM) de la **Colombia de hoy (2026)** al mayor detalle posible —individuos y
hogares reales localizados en los 1.103 municipios— para **traducir programas de
gobierno en intervenciones** y comparar la Colombia que resultaría de cada uno
(ver `docs/OBJETIVO.md`).

Cada agente atraviesa **mecanismos causales con base empírica** (educación, salud,
empleo, delincuencia, demografía; `docs/model-spec/mecanismos.md`), con probabilidades
**sustentadas en datos públicos** (DANE, MEN, GEIH, Medicina Legal). Las políticas son
**palancas intervenibles** por escenario (`experiments/scenarios/`).

## Estado

Pipeline completo funcionando: **población base → mecanismos M1-M7 → escenarios →
comparación**. Primer resultado (2026→2030) en `results/comparacion_escenarios.md`.
Pendiente: calibración fina de niveles absolutos (pobreza/desempleo/Gini), anclar
delincuencia a homicidios municipales, web/API. Diferencias *relativas* entre escenarios
ya son robustas y trazables.

## Quickstart

```bash
# 1) entorno (sin sudo) — provee cmake/compilador/python científico
conda env create -f hpc/environment.yml && conda activate socium

# 2) datos reales (idempotente)
python3 scripts/fetch_divipola.py
python3 scripts/fetch_poblacion_total.py 2026
python3 scripts/fetch_poblacion_municipal.py 2024

# 3) compilar el núcleo
cmake -S core -B build -G Ninja -DCMAKE_BUILD_TYPE=Release && cmake --build build -j

# 4) generar escenarios y simular Colombia 2026->2030
for s in baseline cepeda de_la_espriella; do
  python3 scripts/build_scenario_params.py $s
  ./build/socium_simular --n 4000000 --escenario data/reference/escenario_$s.flat \
      --out experiments/runs/serie_$s.csv
done

# 5) comparar
python3 python/socium/viz/comparar_escenarios.py   # -> results/comparacion_escenarios.md
```

Ejecutables (`build/`): `socium_simular` (simulación temporal), `socium_kwem`
(economía KWEM), `socium_poblacion`, `socium_hogares`, `socium_geo`, `socium_sociedad`.

## Arquitectura (monorepo)

| Carpeta        | Propósito |
|----------------|-----------|
| `core/`        | Motor de simulación ABM en **C++** (HPC, MPI/OpenMP). |
| `bindings/`    | Puente C++ ↔ Python (pybind11). |
| `python/`      | Paquete Python de análisis, visualización y calibración. |
| `experiments/` | Configuraciones de escenarios, corridas y notebooks. |
| `data/`        | Datos empíricos para calibración/validación. |
| `hpc/`         | Scripts de clúster (SLURM), entornos y contenedores. |
| `web/`         | Página web (frontend) para divulgación de resultados. |
| `api/`         | API (backend) para consultar y lanzar escenarios. |
| `docs/`        | Teoría, especificación del modelo (ODD), decisiones de diseño. |
| `articles/`    | Bibliografía: `.bib`, PDFs (Git LFS), notas y por tema. |
| `results/`     | Figuras y tablas curadas para publicaciones. |
| `scripts/`     | Utilidades de línea de comandos. |

## Licencia

Por definir (ver `LICENSE`).
