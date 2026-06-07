# Revisión bibliográfica → diseño del código

Síntesis de la bibliografía en `articles/` y cómo se traduce en el núcleo de Socium.
**Conclusión principal:** toda la literatura converge en un mismo motor —los
**modelos cinéticos de intercambio de riqueza (Kinetic Wealth-Exchange, KWEM)**—
que se extiende naturalmente con redes, políticas (impuestos/redistribución) y
dinámica social. Ese es el backbone computacional con el que empezamos.

---

## A. El backbone económico: intercambio cinético de riqueza

Analogía central (Boltzmann): cuando dos moléculas chocan intercambian energía;
cuando dos agentes transaccionan intercambian riqueza. La riqueza total se conserva
(economía cerrada) salvo que se añadan impuestos/producción.

| Ref | Modelo / aporte | Lo que implementamos |
|-----|-----------------|----------------------|
| `StatMechofMoneyYakovenkot` (Drăgulescu–Yakovenko 2000) | Reparto aleatorio de la riqueza del par → equilibrio **Boltzmann-Gibbs** `P(m)∝e^{-m/T}`, `T=⟨m⟩` | Regla `DY` |
| `chakraborti2000` (Chakraborti–Chakrabarti 2000) | **Propensión al ahorro** λ: cada agente guarda λ·w y reparte el resto → distribución **Gamma** | Regla `SavingPropensity` |
| `patriarca2004` | Forma analítica Gamma: `n = 1 + 3λ/(1−λ)` | Validación/test |
| `ispolatov1998` (Ispolatov–Krapivsky–Redner) | Modelos aditivos/multiplicativos de intercambio; base teórica | Referencia de reglas |
| `boghosian2014` | **Yard-Sale Model** + ecuación de Boltzmann/Fokker-Planck; con impuestos aparece **ley de Pareto** (cutoff inferior + cola de potencia) | Regla `YardSale` + módulo impuestos |
| `Oligarchy_as_a_Phase_Transition` (Boghosian 2015) | Wealth-attained advantage → **transición de fase** a oligarquía (condensación) | Variante futura (sesgo) |
| `bisi2009kinetic_redistribution` (Bisi–Spiga–Toscani) | Impuestos por transacción + **redistribución**; modifica el índice de Pareto | Módulo `tax+redistribution` |
| `journal_pone_0255719` (Banzhaf 2021) | Esquemas concretos de impuestos para amortiguar desigualdad | Catálogo de políticas fiscales |
| `vazquez2025eys_networks` (2025) | **Extended Yard-Sale sobre redes Erdős-Rényi**; criticidad y condensación local | Interacción sobre grafo (no mean-field) |
| `yakovenko2008_econophysics_statmech_review` (RMP 2009) | Review maestro money/wealth/income | Marco general |

**Métrica observable clave:** distribución de riqueza, **coeficiente de Gini**,
fracción de riqueza del top 1%, exponente de Pareto.

## B. Estructura social y redes

| Ref | Aporte | Uso en Socium |
|-----|--------|---------------|
| `zhou2018` | Evolución de red social por **preferential attachment bidireccional** guiado por Pareto | Acoplar topología de red ↔ riqueza |
| `mahault2017` | Desigualdad emergente y **clases sociales auto-organizadas** en red de "poder y frustración" (tipo vidrio de espín) | Módulo de jerarquía/estatus social |
| `galam2012sociophysics` (libro Galam) | Sociofísica de **opinión, voto, fenómenos político-psicológicos** | Módulo de opinión/política |
| `blume1993...` (Blume 1993) | **Mecánica estadística de interacción estratégica** (juegos en lattice, dinámica de Glauber) | Decisiones de agente teoría-de-juegos |

## C. Palancas de política pública (ya presentes en la literatura)

- **Fiscal**: impuestos + redistribución (`bisi2009`, `journal_pone_0255719`,
  `boghosian2014`) → modifican Gini, cutoff y exponente de Pareto.
- **Sesgo/oportunidad**: wealth-attained advantage (`Oligarchy...`) → modela ventaja
  acumulada (los ricos ganan con más probabilidad) = (des)igualdad de oportunidades.
- **Educación/empleo/salud**: aún **sin paper específico** en la colección — pendiente
  de bibliografía (lo modelaremos como modificadores de productividad/tasa de
  interacción cuando lleguen los papers).

## D. Blueprint de HPC (cómo escalarlo)

| Ref | Aporte directo |
|-----|----------------|
| `gill2021` (IEEE TPDS) | **El plano maestro**: modelo económico ABM a escala 1:1 (¡331M agentes en 108 s con 128 cores!), híbrido **MPI+OpenMP**, grafos scale-free. Claves: reducir las comunicaciones aleatorias a un puñado conocido, layout de memoria *cache-efficient* (memory-bound), MPI avanzado. |
| `ICST_SIMUTOOLS2010_8822` (Aaby–Perumalla–Seal, ORNL) | ABM en **multi-GPU + multi-core**, *latency hiding*, CUDA+MPI (×100 speedup) → ruta GPU para la RTX 2080 Ti |
| `ParaCells_IEEE_clean` | Patrones de framework de simulación de agentes en paralelo |

**Implicación para nuestro nodo (Threadripper 64c/128t, 251 GB, 1 GPU):** empezamos
con **OpenMP** (memoria compartida), diseño *struct-of-arrays* cache-friendly, y dejamos
ganchos para MPI (futuro multi-nodo) y CUDA (offload). Coincide con el ADR 0002.

---

## Decisión de implementación (MVP → roadmap)

**MVP (primer código, `core/src/economy`):** motor KWEM con N agentes, reglas
intercambiables (`DY`, `SavingPropensity`, `YardSale`), paso opcional de
**impuesto+redistribución**, y métricas (Gini, top-1%). Paralelización OpenMP por
**barridos con emparejamiento aleatorio sin colisiones** (cada agente en un solo par
por barrido → sin condiciones de carrera).

**Roadmap de extensión (cada capa = un módulo):**
1. Interacción **sobre red** (Erdős-Rényi/scale-free) en vez de mean-field (`vazquez2025`, `gill2021`).
2. **Ventaja acumulada** / igualdad de oportunidades (`Oligarchy...`).
3. **Dinámica social/opinión** (Galam, Blume) acoplada a la economía.
4. Capa de **políticas** parametrizables (educación, empleo, salud) en `core/src/policies`.
5. **MPI** (multi-nodo) y **CUDA** (GPU) según `gill2021` / `ICST_2010`.
