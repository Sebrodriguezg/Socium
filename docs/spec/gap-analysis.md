# Especificación Maestra → Estado del modelo (análisis de brechas)

Mapeo de cada sección del Documento Maestro (`ABM_Colombia_Especificacion_Maestra.docx`)
contra lo que el modelo **ya implementa**, priorizado hacia el objetivo del proyecto
(comparar programas de gobierno 2026-2030, `docs/OBJETIVO.md`).

Leyenda: ✅ hecho · 🟡 parcial · ⛔ falta · 🔵 opcional/avanzado

| Sección spec | Estado | Dónde / qué falta |
|---|---|---|
| §1.1 Ecuación maestra (marco) | 🟡 | El motor es Markov por mecanismos (equivalente operativo); falta formalizar W(X\|X'). |
| §1.2 Geometría riemanniana del territorio (DEM, geodésicas) | ⛔🔵 | Tenemos municipios+coords (DIVIPOLA); falta DEM/tensor métrico. Alto costo, bajo retorno para el objetivo electoral. |
| §1.3 Langevin (SDE de capital) | 🟡 | `economia` (KWEM) mueve riqueza estocásticamente; falta drift μ explícito. |
| §1.4 Fokker-Planck / Pareto | 🟡 | KWEM con ahorro λ genera Gamma/Boltzmann; falta cola de Pareto calibrada (Gini→0.54). |
| §1.5 Decisión Boltzmann (logit) | 🟡 | M5 usa riesgos relativos; migrar a forma logística/partición es directo. |
| §2 Vector de estado individuo (~60 vars) | ✅🟡 | `Population` SoA tiene ~30 vars E; faltan: coords/altitud, comorbilidades, deuda informal, T_ilegalidad, percepción seguridad. |
| §3 Empresas / unidades productivas | ⛔ | No existe agente empresa. Relevante para empleo/extorsión/PIB. |
| §4 Grupos armados (GAO) como agentes | ⛔ | No existe. Clave para M5/conflicto/migración forzada. Falta dato de presencia por municipio. |
| §5.1 Capas geoespaciales (coca, vías, minería) | 🟡 | Tenemos DIVIPOLA + pct_urbano + deserción/cobertura; faltan coca (SIMCI), vías, GAO. |
| §5.2 Macroeconomía (campo externo) | ⛔ | No hay TRM/IPC/precios como variables exógenas. Necesario para cierre micro-macro. |
| §6.1 Mortalidad compuesta (edad+violencia+enfermedad) | 🟡 | Tenemos basal+enfermedad; falta componente violencia/GAO. |
| §6.2 Transiciones laborales (Markov O/D/I) | ✅ | M3 implementado (anual); spec sugiere mensual con moduladores — fácil de afinar. |
| §6.3 Migración por gravedad | ⛔ | M-migración no implementada. Datos: poblacion por municipio ✅, falta red de víctimas. |
| §6.4 P(delinquir) logística 6 predictores | ✅🟡 | M5 con factores de riesgo; faltan β₃ (violencia juvenil), β₅ (GAO), β₆ (impunidad). |
| §6.5 SEIR sobre red | ⛔🔵 | Salud M4 es individual; falta contagio en red. Relevante si se modela epidemia. |
| §6.6 Opinión/radicalización (Ising/voter) | ⛔ | No implementado. Conecta con política/escenarios. |
| §7.1 Replicador espacial (conflicto) | ⛔🔵 | Depende de agentes GAO. |
| §7.2 Teoría de juegos cooperación/deserción | ⛔🔵 | — |
| §7.3 Econofísica intercambio cinético | ✅ | `apply_exchange` (DY/saving/yard-sale) = exactamente §7.3. |
| §8 Iteración temporal (14 pasos) | 🟡 | El motor hace ~8 de los 14 (demografía, educación, laboral, salud, crimen, economía); faltan: macro, GAO, red social, opinión, asimilación. |
| §9 HPC MPI+CUDA | 🟡 | OpenMP ✅ (un nodo); MPI/CUDA preparados (ADR 0002) pero no implementados. Innecesario a la escala actual (1 nodo, 128 hilos). |
| §10 Calibración (DeepIMC/ABC/EnKF) | ⛔🔵 | Calibración manual hoy; automatizar es fase posterior. |
| §11 Brechas (Hamiltoniano, micro-macro) | 🟡 | Brecha micro-macro real y prioritaria; resto avanzado. |

## Lo que YA cumple la spec

- **Vector de estado del agente** (§2) — `Population` SoA, 45 variables E.
- **Econofísica / intercambio cinético** (§7.3) — `apply_exchange`, validado (Gini DY=0.5).
- **Transiciones laborales** (§6.2), **mortalidad** parcial (§6.1), **delincuencia por
  factores de riesgo** (§6.4), **demografía** (§8 paso 11).
- **Geografía real** (§5.1 parcial) — 1.122 municipios DIVIPOLA + población + urbano/rural.
- **HPC OpenMP** (§9 parcial) — 128 hilos, SoA cache-friendly.
- **Motor de iteración temporal** (§8) — 8 de 14 pasos.
- **Capa de políticas/escenarios** — no está en la spec pero es el objetivo del proyecto.

## Prioridad de implementación (alineada al objetivo electoral)

Orientado a que la comparación Cepeda/De la Espriella sea más realista, NO a completar
la spec por completitud. Los 🔵 (geometría riemanniana, SEIR, MPI/CUDA, DeepIMC) se
posponen: alto costo, bajo retorno para el objetivo, o innecesarios a escala de 1 nodo.

1. **Calibración a valores de la spec §6** (estrato, educación, salud, informalidad
   urbano/rural usando pct_urbano que ya tenemos; income con cola de Pareto → Gini 0.54).
   *Ataca la deuda de baseline actual. Bajo riesgo, alto valor.*
2. **Presencia de GAO + impunidad por municipio** (§4, §5.1, §6.4 β₅/β₆) → enriquece M5
   (delincuencia) y diferencia mejor "mano dura" vs "seguridad humana". Requiere dataset.
3. **Cierre micro-macro mínimo** (§11 Brecha 2, §8 paso 12) → que empleo/ingreso agregados
   realimenten el siguiente año. Da coherencia a proyecciones multi-anuales.
4. **Migración por gravedad** (§6.3) → desplazamiento responde a violencia/oportunidad.
5. **Agente empresa** (§3) → empleo y extorsión endógenos.
6. **Opinión/radicalización** (§6.6) → conecta con resultados políticos.

Avanzados pospuestos: §1.2, §6.5, §7.1-7.2, §9 MPI/CUDA, §10 DeepIMC/EnKF.
