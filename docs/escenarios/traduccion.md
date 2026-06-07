# Traducción propuesta → parámetro del motor

Mapeo de **primer orden** entre las palancas de `experiments/scenarios/<id>.yaml` y los
knobs numéricos del motor (`Politicas`). Lo implementa `scripts/build_scenario_params.py`.

> **Estos coeficientes son supuestos de modelado, no hechos del programa.** Son
> transparentes y ajustables. La comparación entre escenarios es más robusta que los
> niveles absolutos (los sesgos del baseline tienden a cancelarse).

| Palanca (yaml) | Knob del motor | Regla de mapeo |
|---|---|---|
| transferencias.monto_per_capita × cobertura | `transfer_ingreso_pc` | `80.000·(monto·cob − 1)` COP/mes pc |
| transferencias.banco_del_pueblo | `transfer_ingreso_pc` | `+20.000` si true |
| transferencias.madres_cabeza_hogar | `transfer_ingreso_pc` | `+8.000` (focalizado) |
| educacion.inversion_pib | `desercion_mult` | `1/inversion` |
| educacion.gratuidad_superior | `desercion_mult` | `×0.85` si true |
| salud.inversion_pib | `salud_prob_mult` | `1/inversion` |
| salud.modelo (liquidación EPS) | `salud_prob_mult` | `×1.05` (disrupción transitoria) |
| salud.inversion_pib > 1 | `aseguramiento_boost` | `0.3` |
| seguridad.enfoque = mano_dura | `crimen_base_mult`, `crimen_abandono_mult` | `0.75`, `0.9` |
| seguridad.enfoque = seguridad_humana | `crimen_base_mult`, `crimen_abandono_mult` | `0.95`, `1.6` |
| laboral.salario_minimo_real | `smlv_mult` | directo |
| laboral.formalizacion | `empleo_mult` | directo |

Efecto en los mecanismos (`docs/model-spec/mecanismos.md`):
`transfer_ingreso_pc` ↑ → menos pobreza, menos deserción (M1) y menos crimen (M5);
`desercion_mult` ↓ → más cobertura educativa (M1) → más ingreso futuro (M2);
`salud_prob_mult` ↓ → menos enfermedad (M4); `crimen_*` → tasa de delincuencia (M5);
`smlv_mult`/`empleo_mult` → ingresos y empleo (M2/M3).

**Pendiente**: refinar coeficientes con evidencia de impacto (evaluaciones de política)
y verificar contra los programas oficiales completos.
