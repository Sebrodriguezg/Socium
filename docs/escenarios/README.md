# Marco de escenarios

Un **escenario** es un conjunto de valores para las **palancas de política** (`P` del
catálogo de métricas) más, opcionalmente, cambios estructurales de reglas. El motor de
escenarios (por construir) lee un `experiments/scenarios/<id>.yaml`, parte de la
**población base 2026 calibrada** y simula el cuatrienio **2026-2030**.

## Escenarios definidos

| Archivo | Descripción |
|---|---|
| `baseline.yaml` | Continuidad: sin cambios de política. Contrafactual de referencia. |
| `cepeda.yaml` | Palancas según el programa de gobierno de **Iván Cepeda**. |
| `de_la_espriella.yaml` | Palancas según el programa de **Abelardo de la Espriella**. |

## Reglas de oro

1. **Neutralidad y simetría**: ambos candidatos se modelan con la **misma estructura**
   de palancas y los **mismos mecanismos**. Lo único que cambia son los valores.
2. **Trazabilidad**: cada palanca con valor distinto del baseline **debe** citar la
   página/sección del **programa oficial** en el campo `fuente`. Nada se inventa.
3. **Supuestos explícitos**: cuando el programa no da una cifra, se usa un supuesto
   declarado en `supuesto:` (rango), no un número arbitrario silencioso.
4. **Incertidumbre**: las palancas cuantitativas admiten rango (min/esperado/max) para
   propagar incertidumbre a las métricas comparadas.

## Anatomía de una palanca

```yaml
fiscal:
  impuesto_renta_top:
    valor: 1.0          # multiplicador vs baseline (1.0 = sin cambio)
    fuente: ""          # cita del programa de gobierno (obligatoria si valor != baseline)
    supuesto: ""        # si el programa no da cifra exacta
    metricas_P: [recaudo_tributario]   # palancas del catálogo que toca
    afecta_A: [coeficiente_de_gini, pobreza_monetaria, gasto_publico_social]
```

## Flujo de trabajo

1. Conseguir el **programa de gobierno** oficial de cada candidato (PDF en `articles/`
   o `docs/escenarios/fuentes/`), con su URL.
2. Llenar `cepeda.yaml` y `de_la_espriella.yaml` palanca por palanca, citando fuente.
3. Correr `baseline`, `cepeda`, `de_la_espriella`.
4. Comparar trayectorias 2026-2030 de las métricas `A` y visualizar.

> Estado: estructura definida; **faltan los programas oficiales** para llenar los
> valores. Ese es el siguiente insumo a conseguir.
