# CONTINUAR — instructivo para retomar el proyecto en un chat nuevo

> **Si eres Claude y empiezas una sesión nueva: LEE ESTE ARCHIVO PRIMERO.** Resume todo el
> estado del proyecto y te devuelve exactamente al punto donde quedamos. Para el humano:
> en un chat nuevo basta con escribir *"Lee `docs/CONTINUAR.md` y retomamos"*.

## 0. Cómo reconstruir el contexto (orden de lectura)
1. Este archivo (`docs/CONTINUAR.md`) — estado y próximos pasos.
2. `docs/consideraciones.md` — retroalimentación del público y qué se implementó.
3. `docs/METODOLOGIA.md` — el modelo completo (agentes, mecanismos, calibración, validación).
4. `git log --oneline -20` — los últimos cambios.
5. (memoria automática de Claude) `MEMORY.md` / `socium-proyecto.md` si están disponibles.

## 1. Qué es Socium (en 3 líneas)
Simulador HPC basado en agentes (ABM) de Colombia 2026, calibrado a DANE, para **comparar
los programas de gobierno de Iván Cepeda y Abelardo de la Espriella** proyectando 2026→2030.
C++20/OpenMP + Python + web estática pública. Trato neutral y simétrico.

- **Repo:** https://github.com/Sebrodriguezg/Socium  ·  **Rama de trabajo:** `develop`
- **Web pública:** https://sebrodriguezg.github.io/Socium/  (se publica con `git subtree push --prefix web origin gh-pages`)
- **Cluster:** Threadripper 3990X, 128 hilos, 251 GB, sin sudo (usa conda).

## 2. Estado actual (al 2026-06-07)
- **Fase A** (competitivos en política): ✅ ensembles + cierre fiscal SFC + validación fuera de muestra 2018-2023.
- **Fase B (Tier 2)**: ✅ red social explícita · ✅ sector financiero (gota a gota) · ✅ producción insumo-producto (Leontief).
- **Consolidación**: ✅ `docs/METODOLOGIA.md` · ✅ stylized facts (5/5) · ✅ web con filtros.
- **Run a 50M agentes (1:1)**: ✅ corrido en el cluster y publicado (commit `datos 50M`).
  - muestra.json ~81k agentes/escenario, 7.359 perfiles. Calibración 2026: gini 0.554,
    desempleo 0.091, informal 0.552, pobreza 0.298. 5/5 stylized facts.
- **Web pública**: consulta rápida por perfil (con fallback anti-cero), consulta avanzada
  multi-filtro (edad, sueldo, sexo, depto, educación, situación, estrato, salud, etnia,
  migrante, zona; muestra la composición del ingreso), panel macro (PIB, inflación, TRM,
  déficit, deuda), mapa coroplético y enlace al formulario de consideraciones.
- **Fase C (Tier 3)**: ⬜ pendiente (asimilación de datos EnKF, expectativas/aprendizaje, emulador ML).

## 3. Consideraciones del formulario (ver `docs/consideraciones.md` para el detalle)
| # | Tema | Estado |
|---|---|---|
| 1 | IPC/costo de vida, intereses, empleo por edad | ✅ Implementada |
| 2 | Fuente del ingreso (laboral/subsidio/ayuda) | ✅ Implementada |
| 3 | Atributos demográficos / pobreza multidimensional (IPM) | ⏳ Pendiente |

**Leer respuestas nuevas (la hoja es pública por enlace):**
```bash
curl -sL "https://docs.google.com/spreadsheets/d/1RlfprtEk8hGivvHPxxbsXbR7qkdZMW5_MhhU_Eobfpc/export?format=csv"
```

## 4. DECISIONES PENDIENTES (aquí quedamos)
1. **Alcance de la consideración #3** (atributos demográficos / IPM). Opciones que estaban sobre la mesa:
   - (A) **Batch rápido + IPM básico** *(recomendado)*: exponer como filtros lo que ya existe
     (tenencia, nº de hijos, SISBÉN, subsidios, estrato) + añadir internet, desplazamiento
     forzado y hacinamiento + un IPM simplificado. Sin re-correr cluster (probar a 6M, luego 50M).
   - (B) **IPM completo (DANE, 15 indicadores)**: requiere descargar/calibrar datos ENCV
     (servicios públicos por tipo, electrodomésticos). Más potente, más lento.
   - (C) **Solo filtros de lo ya existente** (rápido, sin tocar el motor).
   - (D) **Solo registrar** y priorizar otra cosa.
   → *El humano iba a aclarar dudas antes de elegir.*
2. **Re-centrar la pobreza a ~33%**: a 50M quedó en 29.8% (algo baja vs DANE 0.33) tras la
   descomposición de ingreso (#2). Ajuste chico de `ingreso_no_laboral_pc` o del monto de
   ayuda; implica re-correr a 50M.

## 5. Comandos clave
**Compilar y correr local (rápido):**
```bash
g++ -std=c++20 -O3 -fopenmp -Icore/include $(find core/src -name '*.cpp') core/apps/simulate.cpp -o /tmp/socium_simular
/tmp/socium_simular --n 6000000 --years 4 --seed 7 --escenario data/reference/escenario_baseline.flat --out /tmp/serie.csv
```
**Run a escala 1:1 en el cluster (un comando):**
```bash
bash scripts/correr_cluster.sh 50000000 128      # ver hpc/RUNBOOK.md
```
**Generar datos web y publicar:**
```bash
python3 scripts/build_web_data.py
git add web && git commit -m "datos" && git push origin develop
git subtree push --prefix web origin gh-pages
```
**Validar:**
```bash
python3 python/socium/viz/stylized_facts.py
python3 python/socium/viz/validar_historico.py
python3 python/socium/viz/comparar_escenarios.py
```

## 6. Mapa de archivos (lo esencial)
- `core/include/socium/parametros.hpp` — TODOS los parámetros calibrados.
- `core/src/engine/engine.cpp` — el motor (mecanismos M1-M7 + extensiones).
- `core/apps/simulate.cpp` — driver y flags CLI (`--n --years --escenario --out-muestra --out-perfiles ...`).
- `data/reference/*.csv` — datos reales (DANE/DIVIPOLA/conflicto/PIB), vienen con el repo.
- `scripts/` — build_scenario_params.py, build_web_data.py, correr_cluster.sh, fetch_*.py.
- `python/socium/viz/` — validación, comparación, stylized facts.
- `web/index.html` + `web/data/*.json` — la web pública.
- `experiments/scenarios/{baseline,cepeda,de_la_espriella}.yaml` — programas como palancas.
- `docs/consideraciones.md` — log de retroalimentación.

## 7. Cómo arrancar el chat nuevo (pégame esto)
> "Lee `docs/CONTINUAR.md`, `docs/consideraciones.md` y haz `git log --oneline -15`.
> Dime el estado y retomamos donde quedamos (decisión pendiente: alcance de la consideración #3)."
