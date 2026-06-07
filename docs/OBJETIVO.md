# Objetivo del proyecto — Socium · Colombia 2026

## Norte

Construir un **gemelo digital de la Colombia de hoy** (fecha base **2026-06-06**) al
mayor detalle posible, y usarlo para **comparar escenarios de política pública**
derivados de los **programas de gobierno** de los candidatos presidenciales, corriendo
el modelo hacia adelante en el periodo **2026-2030** (cuatrienio presidencial).

Candidatos a modelar (tratamiento **estrictamente neutral y simétrico**):

- **Iván Cepeda**
- **Abelardo de la Espriella**

> Principio de neutralidad: las propuestas de cada candidato se codifican **solo** a
> partir de su **programa de gobierno oficial publicado**, citando la fuente, sin
> interpretación editorial. El modelo no opina; proyecta consecuencias bajo supuestos
> explícitos y auditables.

## Diseño experimental

Tres corridas sobre la **misma población inicial calibrada** (Colombia 2026):

1. **Baseline** — sin cambios de política (continuidad), para tener un contrafactual.
2. **Escenario Cepeda** — palancas según su programa.
3. **Escenario De la Espriella** — palancas según su programa.

Se comparan las **trayectorias 2026→2030** de todas las métricas `A` del catálogo
(Gini, pobreza/IPM, desempleo, informalidad, cobertura educativa/salud, seguridad,
etc.), por municipio/departamento/nación.

## Qué significa "traducir un programa de gobierno a código"

Cadena explícita para cada propuesta:

```
Propuesta del programa  →  palanca P (parámetro/regla)  →  mecanismo del modelo
                        →  métricas A afectadas          →  comparación entre escenarios
```

Ejemplo (ilustrativo, no atribuido):
*"reforma tributaria que sube el impuesto a las rentas altas"* →
`fiscal.impuesto_renta_top` ↑ → módulo de impuesto+redistribución (Bisi/Banzhaf) →
baja Gini e ingreso top 1%, sube recaudo/transferencias → se compara contra baseline.

## Brecha actual → capacidades necesarias (roadmap)

Hoy el modelo tiene: población de individuos + hogares localizados en los 1.103
municipios reales, y un motor de intercambio de riqueza (economía). Para comparar
programas de gobierno de forma creíble necesitamos que el modelo **evolucione en el
tiempo** y que las **políticas conecten con los mecanismos**:

| Bloque | Estado | Necesario para escenarios |
|---|---|---|
| Población + geografía + hogares | ✅ | calibrar a datos reales por municipio |
| Economía (riqueza) | ✅ acoplada | ligar a empleo/ingreso/productividad |
| **Dinámica temporal** (envejecer, nacer/morir, transiciones educativas y laborales) | ⛔ | imprescindible: el cuatrienio se simula año a año |
| **Módulo fiscal** (impuestos, transferencias, gasto) | ⛔ | palanca central de casi todo programa |
| **Educación / salud / empleo** como subsistemas con política | ⛔ parcial (datos) | cada programa interviene aquí |
| **Seguridad / conflicto** | ⛔ | relevante en CO y en los programas |
| **Motor de escenarios** (cargar `scenario.yaml`, aplicar palancas) | ⛔ | ejecutar baseline vs candidatos |
| **Salida de métricas + comparación + viz** | ⛔ | el entregable final |

## Plan reencaminado (orden propuesto)

1. **Calibrar la población base 2026** a datos reales por municipio (pirámide etaria,
   educación, empleo, ingresos) → el "Colombia de hoy" creíble.
2. **Motor de tiempo** (`core/src/engine`): paso anual con demografía (nacimientos,
   muertes, envejecimiento), transiciones educativas y laborales.
3. **Módulo fiscal y de políticas** (`core/src/policies`): impuestos, transferencias,
   gasto sectorial — todo parametrizable.
4. **Motor de escenarios**: leer `experiments/scenarios/*.yaml` y aplicar palancas.
5. **Conseguir y codificar los programas** de Cepeda y De la Espriella (fuentes).
6. **Correr baseline + 2 escenarios**, exportar métricas y **comparar/visualizar**.

## Documentos relacionados

- Métricas: `docs/model-spec/metrics-catalog.md` (roles `P` = palancas).
- Marco de escenarios: `docs/escenarios/README.md`.
- Plantillas: `experiments/scenarios/*.yaml`.
