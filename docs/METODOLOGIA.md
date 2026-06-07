# Socium · Metodología del modelo

Documento integrador del simulador basado en agentes (ABM) de Colombia. Resume el modelo
completo, su calibración, validación e implementación. Para detalle ver: `docs/OBJETIVO.md`,
`docs/model-spec/{ODD,metrics-catalog,mecanismos}.md`, `docs/escenarios/`, `docs/spec/`.

## 1. Objetivo
Gemelo digital de la Colombia 2026 para **comparar escenarios de política pública** —en
particular los programas de gobierno de los candidatos presidenciales— proyectando el
cuatrienio 2026-2030 y midiendo todos los indicadores del estado del país. Tratamiento
neutral y simétrico; las cifras son **escenarios de simulación calibrados**, no predicciones.

## 2. Sustrato de agentes
- **Individuos** (`Population`, SoA): ~45 variables de estado (demografía, educación,
  empleo/informalidad, ingreso, salud, deuda, opinión, migración, geolocalización).
- **Hogares** (`Households`): agrupan individuos del mismo municipio; tipo, ingreso,
  estrato, SISBÉN, tenencia.
- **Empresas** (`Firms`): tamaño (DANE), sector, capital, empleos, formalidad; crean el
  empleo y pagan extorsión en zonas de conflicto.
- **Red social** (`Network`, CSR): capas hogar + comunidad por municipio; soporta efecto
  de pares (crimen) y conformidad (opinión). Escala a 50M agentes (~2.8 GB).

## 3. Geografía y datos reales
Nación → 33 departamentos → 1.103 municipios (DIVIPOLA). Datos integrados (scripts
reproducibles en `scripts/`):
- Población total y urbano/rural por municipio 2026 (DANE proyecciones).
- Educación y deserción por municipio (MEN).
- Homicidios por municipio → índice de conflicto (Policía Nacional).
- PIB per cápita departamental (DANE) → productividad regional.
- Pobreza departamental y serie histórica nacional 2018-2023 (DANE/Banco Mundial).

## 4. Mecanismos causales (paso multi-escala: mensual lo rápido, anual lo lento)
Cada probabilidad se sustenta en datos públicos (ver `docs/model-spec/mecanismos.md` y
`data/reference/parametros.yaml`). Micro-reglas empíricas calibradas a macro-datos reales.
- **M1 Educación**: deserción por estrato/ruralidad (LEE Javeriana, MEN).
- **M2 Ingreso**: Mincer (retorno ~10%/año, GEIH) × productividad regional + dispersión.
- **M3 Empleo**: endógeno a la capacidad de las empresas (emparejamiento departamental).
- **M4 Salud**: enfermar→gasto/productividad (afiliación, ENSIN/SISPRO).
- **M5 Delincuencia**: factores de riesgo (nini, pobreza, baja educación, conflicto local,
  efecto de pares de la red), anclada a homicidios reales por municipio.
- **M6 Movilidad/opinión**: educación como canal; opinión y bienestar por conformidad de red.
- **M7 Demografía**: nacer/envejecer/morir (DANE).
- **Extensiones**: migración por gravedad (huye del conflicto), sector financiero
  (gota a gota), producción insumo-producto (Leontief, 7 sectores), cierre fiscal
  stock-flow consistent (impuestos→gasto→déficit→deuda→penalización de crecimiento).

## 5. Calibración
- **Nacional (DANE 2023)**: pobreza 32%, pobreza extrema ~9%, Gini 0.55, desempleo ~10%,
  informalidad ~55% (búsqueda en malla de calib_ingreso, sigma, elasticidad, empleos_frac).
- **Espacial**: educación rural/urbana + PIB per cápita departamental → reproduce la
  desigualdad regional (Pearson 0.65 vs pobreza departamental real).
- **Fiscal**: baseline déficit ~4%, deuda ~60% (≈ Colombia real).

## 6. Validación
- **Fuera de muestra 2018→2023** (con shock exógeno COVID-2020): reproduce la forma de la
  trayectoria con MAE sin sesgo de 1-2 pp (`results/validacion_historica.md`).
- **Hechos estilizados** (emergentes, no impuestos): top 10% del ingreso 44%, Gini 56%,
  micro-empresas 92% — 5/5 en rango (`results/stylized_facts.md`).
- **Espacial**: ordenamiento de departamentos por pobreza (`results/validacion.md`).

## 7. Incertidumbre
Ensembles Monte Carlo (`scripts/ensemble.py`) perturbando las palancas de política y la
calibración → media + banda predictiva p5-p95. Las conclusiones robustas (p.ej. la
reducción de pobreza de un escenario) se distinguen de las inciertas (p.ej. el desempleo).

## 8. Escenarios de política
Cada programa de gobierno se traduce a **palancas** (`experiments/scenarios/*.yaml`) con
fuente citada y supuestos explícitos; el mapeo palanca→parámetro está en
`docs/escenarios/traduccion.md`. Las palancas modifican los mecanismos (transferencias,
impuestos, gratuidad educativa, enfoque de seguridad, modelo productivo, etc.).

## 9. Implementación (HPC)
C++20 + OpenMP (SoA, cache-friendly). Benchmark: 8M agentes × 4 años = 24 s / 0.5 GB en
16 núcleos → 50M (1:1) ≈ 3 GB / ~2.5 min. El clúster (128 hilos, 251 GB) se aprovecha en
ensembles, sensibilidad y la red social a escala. Salida pública: perfiles agregados →
web estática (`web/`) con consulta por perfil y mapa coroplético.

## 10. Limitaciones (honestas)
- Calibración espacial comprime el rango (enclaves petroleros sesgan el PIB pc).
- Propagación de shocks grandes (COVID) de primer orden: pico de pobreza amortiguado,
  recuperación de empleo lenta.
- Sin asimilación de datos en tiempo real, sin expectativas/aprendizaje (Fase C).
- La traducción programa→palanca usa coeficientes de primer orden, revisables.
- Comparable a JUNE / EURACE-Poledna / Dosi K+S; por debajo en estimación formal y
  cierre macro completo.

## 11. Reproducibilidad
Datos versionados (`data/reference/`), scripts idempotentes (`scripts/`), parámetros con
fuente (`data/reference/parametros.yaml`), build CMake (`core/`). Quickstart en `README.md`.
