# Validación espacial — pobreza por departamento

Modelo (baseline 2026) vs DANE 2023, por departamento.

- Departamentos comparados: **12**
- Correlación de Pearson: **0.653**
- Correlación de Spearman (orden): **0.448**
- Error absoluto medio (MAE): **13.1 pp**
- Rango real: 22%–70%  ·  Rango modelo: 31%–47%

| Departamento | Real DANE | Modelo | Δ |
|---|---|---|---|
| CHOCÓ | 69.9% | 47.2% | -22.7 pp |
| LA GUAJIRA | 67.7% | 42.3% | -25.4 pp |
| SUCRE | 58.1% | 40.6% | -17.5 pp |
| CESAR | 54.4% | 31.8% | -22.6 pp |
| CÓRDOBA | 53.9% | 42.5% | -11.4 pp |
| MAGDALENA | 50.3% | 41.7% | -8.6 pp |
| BOLÍVAR | 49.7% | 32.5% | -17.2 pp |
| CAUCA | 47.0% | 42.6% | -4.4 pp |
| HUILA | 40.2% | 33.6% | -6.6 pp |
| NARIÑO | 38.8% | 42.8% | +4.0 pp |
| CALDAS | 23.6% | 31.4% | +7.8 pp |
| CUNDINAMARCA | 22.4% | 31.5% | +9.1 pp |

**Lectura:** una Spearman alta indica que el modelo ordena bien los departamentos (reproduce el patrón espacial); un MAE/rango menor que el real indica que el modelo **comprime** la desigualdad espacial (driver estructural regional aún por calibrar más allá de educación rural y conflicto).

![scatter](validacion_pobreza.svg)
