# Plan de implementación — Consideración #3 (atributos demográficos / Pobreza Multidimensional)

> Documento para **leer e implementar por fases**. Marca cada `[ ]` al completarlo.
> Origen: respuesta #3 del formulario (laura94cobos, sev 2). Ver `docs/consideraciones.md`.
> Regla de oro: **no romper** la calibración monetaria DANE 2026 ni los 5/5 stylized facts.

## 1. Idea y mapeo
La #3 pide muchos atributos del individuo/hogar (hijos, hogar estable, género/minorías,
desplazamiento forzado, servicios públicos, internet, SISBÉN+puntaje, subsidios, tenencia,
electrodomésticos). En conjunto **es el Índice de Pobreza Multidimensional (IPM) del DANE**
más un par de atributos de vulnerabilidad. Lo construimos como IPM (riguroso y publicable) y
exponemos los atributos como filtros.

Mapeo pedido → componente:
| Pedido | Cómo se modela |
|---|---|
| ¿tiene hijos? ¿cuántos? | composición del hogar (menores en `Households`) |
| ¿hogar estable? / hacinamiento | hacinamiento crítico (personas/cuarto) — IPM |
| género/minorías | añadir categoría de género diverso (representación) |
| desplazamiento forzado | flag por agente, atado a municipios de conflicto |
| servicios públicos (agua/excretas) | indicadores de vivienda — IPM |
| acceso a internet | atributo del hogar (ENCV) |
| SISBÉN + puntaje | ya en `Households` (SISBÉN); añadir puntaje/grupo A-D |
| beneficiario de subsidio | ya existe (transferencias/subsidios) |
| ¿casa propia o arriendo? | tenencia (ya en `Households`) — exponer |
| electrodomésticos | proxy de bienestar material (ENCV) — opcional |

## 2. El IPM del DANE (definición a implementar)
5 dimensiones (20% c/u); un hogar es **multidimensionalmente pobre** si la suma de privaciones
ponderadas ≥ **33.3%**. Indicadores y si ya están en el modelo:

| Dimensión (peso) | Indicador | Peso | ¿Ya en el modelo? |
|---|---|---|---|
| Educación (20%) | Bajo logro educativo | 10% | ✅ (nivel educativo del hogar) |
| | Analfabetismo | 10% | ✅ (nivel/escolaridad) |
| Niñez y juventud (20%) | Inasistencia escolar | 5% | ✅ (`asiste_escuela`) |
| | Rezago escolar | 5% | ✅ (escolaridad vs edad) |
| | Barreras primera infancia | 5% | ➕ asignar (estrato/zona) |
| | Trabajo infantil | 5% | ➕ asignar (estrato/zona) |
| Trabajo (20%) | Desempleo larga duración | 10% | ✅ aprox. (`situacion_laboral`) |
| | Empleo informal | 10% | ✅ (`informal`) |
| Salud (20%) | Sin aseguramiento | 10% | ✅ (`afiliacion_salud==Ninguno`) |
| | Barreras acceso salud | 10% | ➕ asignar (estrato/zona) |
| Vivienda y SS.PP. (20%) | Sin agua mejorada | 5% | ➕ NUEVO (hogar) |
| | Inadecuada eliminación excretas | 5% | ➕ NUEVO (hogar) |
| | Pisos inadecuados | 5% | ➕ NUEVO (hogar) |
| | Paredes inadecuadas | 5% | ➕ NUEVO (hogar) |
| | Hacinamiento crítico | 5% | ➕ NUEVO (hogar) |

**Objetivos de calibración (DANE IPM 2023):** nacional ≈ **12%**, cabeceras (urbano) ≈ 8%,
centros poblados y rural disperso ≈ **27%**. El motor de privaciones de vivienda/servicios se
asigna por **estrato y zona** para reproducir esa brecha urbano-rural.

## 3. Datos necesarios
Crear `data/reference/privaciones_ipm.csv` con la tasa de privación por **estrato (1-6) × zona
(urbano/rural)** para los indicadores que hay que asignar (vivienda, servicios, barreras,
trabajo infantil). Cifras de referencia (DANE ENCV/IPM 2022-2023; refinar con la fuente):

| Indicador | estrato1 urb | estrato1 rural | estrato3 urb | estrato5 urb |
|---|---|---|---|---|
| Sin agua mejorada | 0.10 | 0.40 | 0.02 | 0.00 |
| Inadecuada excretas | 0.12 | 0.45 | 0.02 | 0.00 |
| Pisos inadecuados | 0.10 | 0.30 | 0.01 | 0.00 |
| Paredes inadecuadas | 0.04 | 0.12 | 0.005 | 0.00 |
| Hacinamiento crítico | 0.20 | 0.25 | 0.05 | 0.01 |
| Barreras primera infancia | 0.15 | 0.30 | 0.05 | 0.01 |
| Trabajo infantil | 0.04 | 0.10 | 0.01 | 0.00 |
| Barreras acceso salud | 0.10 | 0.18 | 0.05 | 0.02 |
| Sin internet | 0.45 | 0.80 | 0.15 | 0.02 |

(Si se consigue ENCV por departamento, mejor: `scripts/fetch_ipm.py`. Si no, esta tabla por
estrato/zona es suficiente para reproducir la incidencia y la brecha urbano-rural.)

## 4. Fases (implementar en orden; cada una compila y verifica)

### Fase 0 — Datos `[ ]`
- [x] `data/reference/privaciones_ipm.csv` con la tabla de §3.
- [ ] (Opcional) `scripts/fetch_ipm.py` para traer ENCV/IPM por departamento de datos.gov.co.
- [ ] Cargar la tabla en `Geography` o en un nuevo `Parametros` (mapa estrato×zona→tasas).

### Fase 1 — Atributos materiales del hogar `[ ]`
- [x] En `Households` (o `Population`) añadir flags por hogar: `sin_agua`, `sin_excretas`,
      `piso_inadecuado`, `pared_inadecuada`, `hacinamiento`, `sin_internet`,
      `barrera_priminfancia`, `trabajo_infantil`, `barrera_salud`. (uint8, 1 = privado.)
- [x] Asignación al construir los hogares: `prob = privaciones_ipm[estrato][zona]` y sortear
      cada flag (correlacionados con estrato/zona). Recordar `resize`/`push_back`/init.
- [x] Exponer también lo que ya existe: `tenencia` (propia/arriendo), `n_hijos` (menores del
      hogar), `sisben_grupo` (A-D desde el puntaje).

### Fase 2 — Cálculo del IPM `[ ]`
- [x] En `engine.cpp`, función `calcular_ipm()` (anual): por hogar, sumar pesos de las
      privaciones (los ✅ se derivan del estado; los ➕ de los flags) → `privacion_pct`.
      Hogar pobre multidimensional si `privacion_pct ≥ 1/3`.
- [x] Métrica nacional `ipm` (y `ipm_urbano`, `ipm_rural`) en `MetricasAnuales` + CSV + serie.
- [x] **Calibrar** las tasas de §3 hasta IPM nacional ≈ 12%, rural ≈ 27%, urbano ≈ 8%.
- [x] Verificar que pobreza monetaria, Gini y 5/5 stylized facts NO se rompen.

### Fase 3 — Web (filtros + visualización) `[ ]`
- [x] `exportar_muestra`: añadir columnas `ipm_pobre`, `tenencia`, `n_hijos`, `internet`,
      `sisben`, `desplazado`, y la composición de privaciones.
- [x] `build_web_data.py`: incluir los nuevos campos en `muestra.json`.
- [x] `web/index.html` (consulta avanzada): filtros nuevos (tenencia, hijos, internet, SISBÉN,
      desplazado) + mostrar **"Pobreza multidimensional (IPM)"** y el desglose de privaciones.
- [x] Panel macro / mapa: añadir IPM como variable seleccionable (urbano vs rural).

### Fase 4 — Atributos de vulnerabilidad extra `[ ]`
- [x] `desplazamiento_forzado`: flag por agente, prob. atada a `mpio_conflicto` (víctimas).
- [ ] Género: añadir categoría diversa (representación de minorías) — pequeña fracción.
- [ ] SISBÉN puntaje/grupo refinado (A-D) y su uso en focalización de subsidios.

### Fase 5 — Validación y publicación `[ ]`
- [ ] `python/socium/viz/` : chequeo de IPM (nacional + brecha urbano-rural) vs DANE.
- [ ] Re-correr a 50M en el cluster (`scripts/correr_cluster.sh 50000000 128`).
- [ ] `git push origin develop` + `git subtree push --prefix web origin gh-pages`.
- [ ] Actualizar `docs/consideraciones.md` (#3 → ✅) y `docs/CONTINUAR.md`.

## 5. Alcance recomendado (si hay que priorizar)
- **Mínimo viable (A):** Fases 0-3 con la tabla por estrato/zona → IPM funcional + filtros.
  No requiere descargar datos nuevos; reproduce la brecha urbano-rural. *(recomendado para empezar)*
- **Completo (B):** + Fase 4 + `fetch_ipm.py` con ENCV por departamento (más fino y publicable).

## 6. Riesgos / cuidados
- El IPM es **independiente** de la pobreza monetaria (no tocar `linea_pobreza`): se reporta
  como dimensión aparte (un hogar puede ser pobre monetario y no multidimensional, y viceversa).
- Asignar privaciones correlacionadas con estrato/zona, no aleatorias puras (si no, la brecha
  urbano-rural no aparece).
- Cada campo nuevo en `Population/Households` exige tocar `resize`, `nacer/push_back` y la
  construcción sintética (es el error típico; ver historial).
- Mantener `muestra.json` manejable; si crece mucho, subir `--muestra-paso` o usar Git LFS.
