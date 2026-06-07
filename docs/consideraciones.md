# Registro de consideraciones (retroalimentación del formulario)

Respuestas del público (formulario) y cómo se incorporaron al modelo.

## #1 — 2026-06-07 · yicedgarcia24@icloud.com
**Categoría:** Economía / trabajo / informalidad · **Severidad:** 3 (sesgo importante)
**Falta:** tener en cuenta el incremento del IPC y del salario mínimo, y el **costo de vida**
asociado; cómo se ven influenciados los **créditos e intereses** de la persona; y la
**probabilidad de conseguir empleo según la edad**.

**Incorporado (commit de macro/IPC):**
- **Costo de vida / IPC**: el motor calcula inflación y nivel de precios; la inflación por
  encima de la meta **erosiona el ingreso real de los informales** (canal IPC→pobres). El
  salario mínimo es palanca (`smlv_mult`). Panel macro muestra IPC, TRM, costo de vida.
- **Créditos e intereses**: la tasa de interés (formal y gota a gota) **sube con la
  inflación** (prima Banrep) → más estrés financiero cuando sube el IPC.
- **Empleo por edad**: la probabilidad de conseguir empleo ahora depende de la edad
  (juvenil 15-24 más difícil, mayores 55+ más difícil) → reproduce el desempleo juvenil real.

**Pendiente / a profundizar:** salario mínimo real diferenciado por sector; indexación de
arriendos/servicios al IPC; curva de empleo por edad calibrada con GEIH.
