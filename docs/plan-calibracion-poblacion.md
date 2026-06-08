# Plan — Calibrar la población sintética a la distribución real (DANE)

> Documento para **leer e implementar por fases**. Origen: el usuario detectó que cambiar UN
> solo campo (estudiante → cuenta propia) lleva un perfil de 9 agentes a 0, siendo ambos casos
> comunes en la realidad. Marca cada `[ ]` al completarlo.

## 1. El problema (raíz)
`build_synthetic` (core/src/agents/population.cpp) genera la mayoría de atributos con sorteos
**independientes** (sector, posición, informalidad, afiliación, etnia, etc. cada uno por su
cuenta; educación apenas ligada a la edad; ingreso lognormal por educación). Resultado: la
distribución conjunta ≈ **producto de marginales**, que NO es la realidad. La realidad tiene
correlaciones fuertes (cuenta propia ↔ informal ↔ baja educación ↔ estrato bajo; régimen
subsidiado ↔ bajo ingreso; jóvenes ↔ menos cuenta propia; etc.). Por eso:
- Cada filtro corta casi independientemente → al combinar ~16 filtros, celdas reales quedan
  vacías (0 agentes) y celdas implausibles quedan pobladas. **Evidencia:** joven 15-24,
  Bogotá, media, estrato 3, SISBÉN D, con hijos, arriendo, cuenta propia, <$1M → 0; idéntico
  pero "estudiante" → 9. Ambos son comunes; el modelo no lo refleja.

**Meta:** que la proporción de cada caso (y sus cruces) coincida con el DANE, y que **ningún
perfil común dé 0**.

## 2. Enfoques (de más a menos riguroso)
- **(A) Resampleo de microdatos GEIH/Censo** *(gold standard, Fase 2)*: cada agente = vector de
  atributos de una persona real (anonimizada) de la GEIH, reponderado a la población municipal.
  Garantiza la conjunta real. Requiere bajar microdatos DANE (ANDA) y mapear variables; vivienda/
  SISBÉN desde Censo 2018/ENCV (matching estadístico si hace falta).
- **(B) Generación CONDICIONAL en orden causal** *(recomendado para empezar, Fase 1)*: reescribir
  `build_synthetic` para sortear los atributos **condicionados** unos a otros con probabilidades
  del DANE, en vez de independientes. Captura las correlaciones principales sin microdatos
  individuales. Factible ya, offline.
- **(C) IPF / reponderación** a marginales y cruces publicados: complementa (B) ajustando pesos.

## 3. Orden causal propuesto para (B)  [el corazón de la Fase 1]
Sortear cada atributo condicionado a los anteriores, con tablas DANE (GEIH/Censo/ENCV):
1. **Geografía** (municipio, urbano/rural) — ya está (ponderado por población).
2. **edad, sexo** — pirámide DANE por municipio.
3. **educación | edad, zona** — nivel educativo según edad y urbano/rural (GEIH).
4. **situación laboral | edad, educación, sexo** — P(ocupado/desocupado/inactivo/estudiante)
   por grupo (GEIH). Aquí se fija que jóvenes estudian más, etc.
5. **posición | situación, educación, edad** — asalariado / cuenta propia / empleador / sin
   remuneración (GEIH: cuenta propia sube con edad y baja educación).
6. **informalidad | posición, educación, sector** — informal ~85% en cuenta propia, ~40% asalariado.
7. **ingreso | educación, posición, informalidad, edad, región** — Mincer + dispersión (ya casi).
8. **estrato | región, ingreso del hogar** — distribución de estrato por municipio (Censo/catastro).
9. **SISBÉN | estrato, ingreso** — correlacionado, con dispersión (ya hecho ±1).
10. **afiliación salud | informalidad, ingreso** — subsidiado ↔ informal/bajo ingreso; contributivo
    ↔ formal (hoy es casi independiente → arreglar: subsidiado no debería ser estrato 5-6).
11. **vivienda/servicios/IPM | estrato, zona** — ya por estrato×zona.
12. **etnia, migrante, desplazado** | región — etnia concentrada por región (NARP Pacífico, etc.).

## 4. Datos necesarios
- **GEIH microdatos** (DANE/ANDA): edad, sexo, educación, situación, posición, informalidad,
  ingreso, región. `scripts/fetch_geih.py` (a crear) o tablas de conindicionales pre-extraídas.
- **Censo 2018 / proyecciones**: pirámide por municipio, educación, vivienda, estrato.
- **ENCV**: servicios públicos, internet, electrodomésticos (para IPM, ya aproximado).
- Guardar las **tablas de probabilidad condicional** en `data/reference/cond_*.csv`.

## 5. Fases
### Fase 0 — Datos `[ ]`
- [ ] Conseguir GEIH (microdatos o cruces) y extraer las conditionales clave de §3 a
      `data/reference/cond_*.csv` (situación|edad,educación; posición|situación,educación,edad;
      informal|posición; afiliación|informal,ingreso; estrato|región).
### Fase 1 — Generación condicional `[ ]`
- [ ] Reescribir `build_synthetic` para sortear en el orden causal de §3 leyendo las tablas.
- [ ] Sustituir las independencias actuales (sector/posición/informal/afiliación) por
      condicionales. Mantener edad-educación-ingreso (ya razonable) y mejorarlas.
- [ ] Re-calibrar marginales a DANE 2023 (pobreza 33%, Gini .55, desempleo 10%, informal 55%).
### Fase 2 — Microdatos (opcional, gold) `[ ]`
- [ ] `scripts/fetch_geih.py`; seed de agentes por resampleo de registros reales reponderados.
### Fase 3 — Validación `[ ]`
- [ ] Comparar cruces del modelo vs GEIH (situación×edad, posición×educación, etc.).
- [ ] **Test anti-cero**: una batería de perfiles comunes (joven cuenta propia, madre cabeza de
      hogar informal, adulto formal estrato 3, etc.) debe dar conteos >0 razonables.
- [ ] 5/5 stylized facts y DANE intactos. Re-correr 50M en el cluster y publicar.

## 6. Recomendación
Empezar por **Fase 1 (generación condicional)** con las conditionales de la GEIH: es el mayor
salto de realismo y arregla el problema reportado, sin depender de bajar microdatos pesados.
Fase 2 (resampleo GEIH) después, si se quiere el máximo rigor.

## 7. Riesgos / cuidados
- Reescribir `build_synthetic` cambia TODA la población → re-verificar las 5 métricas DANE y los
  stylized facts; recalibrar parámetros (ingreso, empleos_frac, etc.).
- Mantener los sorteos nuevos **deterministas o con RNG controlado** para no desviar la
  calibración sensible del empleo (gotcha conocido).
- Es un cambio grande → requiere re-correr 50M en el cluster al final.
