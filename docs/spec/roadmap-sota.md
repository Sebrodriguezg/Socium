# Hoja de ruta hacia el estado del arte (ABM nacional)

Análisis honesto: qué tienen los ABM de escala nacional de referencia (JUNE, EURACE/
Poledna, Dosi "K+S", gill2021, OpenABM-Covid) que aún no tenemos, y en qué orden
implementarlo para ser competitivos **en nuestro caso de uso** (comparar programas de
gobierno sobre una Colombia calibrada, con acceso público).

## Dónde YA estamos al nivel
- Población sintética 1:1 viable (50M en ~3 GB / pocos min) — escala JUNE.
- Multi-dominio acoplado (economía, empleo, educación, salud, crimen, migración, opinión).
- Econofísica de riqueza (KWEM) — Yakovenko/Chakrabarti.
- Calibración a agregados nacionales DANE (pobreza, Gini, desempleo, informalidad) +
  validación espacial por departamento.
- Reproducibilidad: datos versionados, scripts idempotentes, ODD iniciado, acceso web.

## Brechas vs. estado del arte (con esfuerzo y si usa cluster)

### Tier 1 — imprescindibles para credibilidad de política (hacer primero)
1. **Cuantificación de incertidumbre** (ensembles Monte Carlo + intervalos de confianza +
   análisis de sensibilidad Sobol). Hoy reportamos una sola corrida sin barras de error.
   *Esfuerzo: bajo-medio. Cluster: sí (cientos de corridas en paralelo).*
2. **Cierre macro contable stock-flow consistent (SFC)**: cuentas cerradas de hogares,
   empresas, gobierno y resto del mundo; impuestos = transferencias + gasto + déficit →
   deuda; dinero endógeno. Hoy el macro es un ciclo+productividad reducido. Sin esto, un
   programa que "sube transferencias" no paga su costo fiscal → las comparaciones no son
   presupuestariamente creíbles. *Esfuerzo: alto. Cluster: no.*
3. **Calibración y validación formal**: estimación por momentos (SMM) o bayesiana (ABC/BSL)
   contra muchos momentos, y **validación fuera de muestra** (reproducir 2018→2023 con
   datos históricos). Hoy es calibración manual por malla a pocos agregados.
   *Esfuerzo: medio-alto. Cluster: sí (miles de evaluaciones).*

### Tier 2 — realismo estructural (siguiente)
4. **Red social explícita** (capas hogar/trabajo/escuela/vecindario) en vez de
   campo-medio/emparejamiento departamental: contagio, crimen, opinión y búsqueda de
   empleo sobre la red real (Barabási-Albert/geográfica). *Esfuerzo: alto. Cluster: sí
   (memoria, matriz dispersa ~60 GB a 50M) — aquí entran los 251 GB.*
5. **Sector financiero**: bancos, crédito, mora y cascadas de default; deuda de hogares y
   empresas endógena. *Esfuerzo: alto. Cluster: no.*
6. **Producción + insumo-producto sectorial** (estilo Dosi K+S): funciones de producción,
   inversión, precios, inventarios, cadenas entre sectores/CIIU. Hoy las empresas son
   puestos+extorsión. *Esfuerzo: alto. Cluster: no.*

### Tier 3 — frontera
7. **Asimilación de datos / nowcasting** (Ensemble Kalman Filter, filtro de partículas)
   para corregir trayectorias con datos nuevos (Poledna lo hace en tiempo real).
8. **Expectativas y aprendizaje** de los agentes (adaptativo / aprendizaje por refuerzo).
9. **Emulador ML** (tipo DeepIMC, BiLSTM) para calibración rápida y para servir la web sin
   recomputar.

## Stylized facts a validar (criterio de "competitivo")
Un ABM macro SOTA reproduce SIN forzar: cola de Pareto de riqueza, Zipf de tamaño de
empresas, curva de Phillips, curva de Beveridge, ciclos económicos endógenos, persistencia
del desempleo. Debemos añadir tests que verifiquen su emergencia.

## Orden recomendado
**Fase A (competitivos en política, semanas):** Tier 1 → (1) ensembles+CI, (2) cierre
fiscal SFC mínimo, (3) validación fuera de muestra 2018-2023. Con esto las comparaciones
Cepeda/De la Espriella pasan de "ilustrativas" a "defendibles".
**Fase B (realismo, 1-2 meses):** Tier 2 → red social + sector financiero + producción I-O.
**Fase C (frontera):** Tier 3 → asimilación de datos, aprendizaje, emulador.

> Nota: alcanzar EURACE-completo (SFC estimado) es trabajo de meses; pero un subconjunto
> enfocado (Fase A) ya nos hace genuinamente competitivos para el objetivo electoral.
