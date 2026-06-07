# Validación espacial — pobreza por departamento

Modelo (baseline 2026) vs DANE 2023, por departamento.

- Departamentos comparados: **12**
- Correlación de Pearson: **0.466**
- Correlación de Spearman (orden): **0.350**
- Error absoluto medio (MAE): **18.5 pp**
- Rango real: 22%–70%  ·  Rango modelo: 29%–36%

| Departamento | Real DANE | Modelo | Δ |
|---|---|---|---|
| CHOCÓ | 69.9% | 31.3% | -38.6 pp |
| LA GUAJIRA | 67.7% | 36.4% | -31.3 pp |
| SUCRE | 58.1% | 29.4% | -28.7 pp |
| CESAR | 54.4% | 28.8% | -25.6 pp |
| CÓRDOBA | 53.9% | 33.7% | -20.2 pp |
| MAGDALENA | 50.3% | 33.5% | -16.8 pp |
| BOLÍVAR | 49.7% | 29.9% | -19.8 pp |
| CAUCA | 47.0% | 35.2% | -11.8 pp |
| HUILA | 40.2% | 29.8% | -10.4 pp |
| NARIÑO | 38.8% | 31.7% | -7.1 pp |
| CALDAS | 23.6% | 28.7% | +5.1 pp |
| CUNDINAMARCA | 22.4% | 29.1% | +6.7 pp |

**Lectura:** una Spearman alta indica que el modelo ordena bien los departamentos (reproduce el patrón espacial); un MAE/rango menor que el real indica que el modelo **comprime** la desigualdad espacial (driver estructural regional aún por calibrar más allá de educación rural y conflicto).

![scatter](validacion_pobreza.svg)
