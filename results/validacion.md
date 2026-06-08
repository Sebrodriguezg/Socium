# Validación espacial — pobreza por departamento

Modelo (baseline 2026) vs DANE 2023, por departamento.

- Departamentos comparados: **12**
- Correlación de Pearson: **0.565**
- Correlación de Spearman (orden): **0.434**
- Error absoluto medio (MAE): **13.8 pp**
- Rango real: 22%–70%  ·  Rango modelo: 32%–48%

| Departamento | Real DANE | Modelo | Δ |
|---|---|---|---|
| CHOCÓ | 69.9% | 48.1% | -21.8 pp |
| LA GUAJIRA | 67.7% | 38.2% | -29.5 pp |
| SUCRE | 58.1% | 42.8% | -15.3 pp |
| CESAR | 54.4% | 31.7% | -22.7 pp |
| CÓRDOBA | 53.9% | 41.9% | -12.0 pp |
| MAGDALENA | 50.3% | 38.7% | -11.6 pp |
| BOLÍVAR | 49.7% | 32.0% | -17.7 pp |
| CAUCA | 47.0% | 40.4% | -6.6 pp |
| HUILA | 40.2% | 34.9% | -5.3 pp |
| NARIÑO | 38.8% | 43.9% | +5.1 pp |
| CALDAS | 23.6% | 32.4% | +8.8 pp |
| CUNDINAMARCA | 22.4% | 31.7% | +9.3 pp |

**Lectura:** una Spearman alta indica que el modelo ordena bien los departamentos (reproduce el patrón espacial); un MAE/rango menor que el real indica que el modelo **comprime** la desigualdad espacial (driver estructural regional aún por calibrar más allá de educación rural y conflicto).

![scatter](validacion_pobreza.svg)
