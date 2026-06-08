# Registro de consideraciones (retroalimentación del formulario)

Respuestas del público (formulario) y cómo se incorporaron al modelo.

**Formulario:** https://forms.gle/FMZbfYk64mKCBCQz8
**Hoja de respuestas (pública por enlace):** https://docs.google.com/spreadsheets/d/1RlfprtEk8hGivvHPxxbsXbR7qkdZMW5_MhhU_Eobfpc/edit
**Leer las respuestas (sin login, export CSV):**
`curl -sL "https://docs.google.com/spreadsheets/d/1RlfprtEk8hGivvHPxxbsXbR7qkdZMW5_MhhU_Eobfpc/export?format=csv"`

## Estado
| # | Fecha | Categoría | Severidad | Estado |
|---|---|---|---|---|
| 1 | 07-jun-2026 | Economía: IPC/costo de vida, intereses, empleo por edad | 3 | ✅ Implementada |
| 2 | 07-jun-2026 | Economía: fuente del ingreso (laboral/subsidio/ayuda) | 3 | ✅ Implementada |
| 3 | 07-jun-2026 | Demografía: atributos y pobreza multidimensional (IPM) | 2 | ✅ Implementada (alcance A) |


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

## #2 — 2026-06-07 · sr260035@gmail.com
**Categoría:** Economía / trabajo / informalidad · **Severidad:** 3 (sesgo importante)
**Inconsistencia:** una persona sin empleo aparece con ingresos sin que se sepa de dónde;
falta implementar la FUENTE del ingreso: laboral, subsidio, o solo probabilidad de caridad.

**Incorporado:**
- **Descomposición del ingreso per cápita por fuente**: laboral (trabajo propio + de otros
  miembros del hogar), subsidio (pensión/Colombia Mayor + transferencias de política), y
  otro (remesas/rentas + ayuda informal). El motor calcula y exporta `pc_lab/pc_sub/pc_otro`.
- **Ayuda/caridad probabilística**: la ayuda informal/rebusque ya NO es automática para todo
  no-ocupado; cada uno tiene una probabilidad (`prob_ayuda_informal`) de recibirla, con un
  monto cuando la recibe -> algunos quedan sin nada (más realista).
- La **web** muestra ahora la composición ("· de trabajo / · de subsidios/pensión /
  · de hogar/ayuda/otros") -> queda claro por qué un desempleado tiene (o no) ingresos.
- DANE 2026 intacto (Gini 0.555, desempleo 10.5%, informal 0.551), 5/5 stylized facts.

## #3 — 2026-06-07 · laura94cobos@gmail.com
**Categoría:** Individuo / demografía · **Severidad:** 2 (mejoraría la precisión notablemente)
**Falta (atributos/condiciones materiales del agente):** ¿tiene hijos? ¿cuántos?; ¿hogar
estable?; género con opciones para minorías; ¿desplazamiento forzado?; acceso a servicios
públicos (cuáles); acceso a internet; SISBÉN y puntaje; % de pertenencia a comunidad
minoritaria/vulnerable; ¿beneficiario de subsidio? cuál (selección múltiple); ¿casa propia
o arriendo?; electrodomésticos del hogar (TV, nevera, estufa, licuadora, computador, teléfono).

**Análisis (esto es esencialmente la POBREZA MULTIDIMENSIONAL — IPM/DANE):**
- Ya en el agente/hogar (solo falta exponer como filtro): tenencia (propia/arriendo),
  nº de hijos (composición del hogar), SISBÉN, estrato, subsidios.
- Cheap de añadir (flag + asignación por estrato/zona/ENCV): acceso a internet, desplazamiento
  forzado (atar a municipios de conflicto), hacinamiento (hogar estable).
- Requiere datos (ENCV/IPM): servicios públicos por tipo, electrodomésticos -> construir el
  **Índice de Pobreza Multidimensional (IPM, 15 indicadores/5 dimensiones, DANE)**.

**Incorporado (alcance A — Fases 1-4 del plan):**
- **Índice de Pobreza Multidimensional (IPM, metodología DANE)**: `engine.calcular_ipm()` con
  5 dimensiones (educación, niñez, trabajo, salud, vivienda); pobre si ≥1/3 de privaciones
  ponderadas. Indicadores de educación/trabajo/salud endógenos; vivienda/niñez por flags del
  hogar asignados por estrato×zona (`data/reference/privaciones_ipm.csv`).
- **Atributos del hogar** expuestos: tenencia (propia/arriendo), nº de hijos, internet,
  SISBÉN, + privaciones materiales (agua, excretas, pisos, paredes, hacinamiento).
- **Desplazamiento forzado**: flag por agente atado al conflicto del municipio (~16.5%).
- **Métricas** `ipm`, `ipm_urbano`, `ipm_rural` en la serie; **web**: panel macro con IPM y
  consulta avanzada con "pobreza multidimensional" + filtros (vivienda, internet, ¿hijos?,
  SISBÉN, ¿desplazado?).
- Calibración: IPM nacional 12.4% (DANE ~12%), urbano 6.8%, rural ~31-34% (gap realista). DANE
  monetario intacto (pobreza 0.30, Gini 0.555), 5/5 stylized facts. Cepeda baja IPM (9.5% vs 10.9%).

**Pendiente (alcance B, opcional):** afinar rural a ~27% y diferenciación IPM de desplazados;
descargar ENCV por departamento (`scripts/fetch_ipm.py`); género diverso; electrodomésticos;
re-correr a 50M en el cluster y publicar. Ver `docs/plan-consideracion-3.md`.
