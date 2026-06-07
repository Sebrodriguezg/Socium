# Mecanismos causales del modelo — Socium · Colombia

El modelo NO usa correlaciones genéricas: cada agente, cada año simulado, atraviesa
**transiciones probabilísticas** cuyos parámetros están **sustentados en datos públicos
colombianos** (o, donde no exista una probabilidad medida directamente, se modela con
**factores de riesgo relativos** y se **calibra para reproducir el agregado real**).

Convención de cada mecanismo:
- **Evento** del agente.
- **Entradas**: variables de estado (`E`) que lo condicionan.
- **Forma funcional**: cómo se calcula la probabilidad.
- **Parámetros + fuente**: valores reales (ver `data/reference/parametros.yaml`).
- **Calibración macro**: indicador agregado (`A`) real al que se ajusta.

Principio: **micro-reglas con base empírica → calibradas a macro-datos reales**. Toda
probabilidad sin medición directa se marca como **supuesto** y se ancla a un agregado.

---

## M1 — Trayectoria educativa (continuar vs desertar)

- **Evento**: cada año escolar, un estudiante continúa o **deserta**.
- **Entradas**: estrato/SISBÉN del hogar, edad, nivel educativo de los padres,
  ingreso del hogar, ruralidad, municipio.
- **Forma funcional**: `p_desercion = base_nivel · RR_estrato · RR_padres · RR_rural`,
  acotada y centrada en la deserción real del municipio.
- **Parámetros + fuente**: deserción superior **estrato 1 = 13,3%** vs **estrato 6 =
  7,1%** (LEE, Javeriana 2021); deserción escolar (5-16) **~3,7%** nacional y la
  **deserción real por municipio** que ya tenemos (`data/reference/municipios.csv`,
  MEN). Mayor peso: privaciones del contexto socioeconómico, trabajo infantil, baja
  educación de los padres.
- **Calibración macro**: tasa de deserción por municipio (MEN) y cobertura neta.

## M2 — Logro educativo → ingreso (retornos a la educación)

- **Evento**: el ingreso laboral depende del capital humano acumulado.
- **Entradas**: años de escolaridad, experiencia (edad), formalidad, sexo, región.
- **Forma funcional**: Mincer — `ln(ingreso) = β0 + β_educ·años + β_exp·exp + ...`.
- **Parámetros + fuente**: retorno por año de escolaridad **≈ 9,5% (1985) → 11,3%
  (2000)**, ~10-14% recientes (estudios con GEIH/DANE). Brecha salarial de género y
  penalización por informalidad reales (GEIH).
- **Calibración macro**: distribución de ingresos y Gini (DANE/GEIH) por departamento.

## M3 — Mercado laboral (empleo, desempleo, informalidad)

- **Evento**: cada año, transición ocupado ↔ desocupado ↔ inactivo; y formal ↔ informal.
- **Entradas**: educación, edad, sexo, sector, ciclo económico, municipio.
- **Forma funcional**: probabilidades de transición condicionadas; informalidad mayor
  a menor educación y en cuenta propia.
- **Parámetros + fuente**: desempleo ~**10%**, informalidad ~**56-58%** (GEIH); por
  educación y posición ocupacional.
- **Calibración macro**: desempleo e informalidad por municipio/departamento (GEIH).

## M4 — Salud (enfermar → productividad → ingreso)

- **Evento**: cada año, el agente puede **enfermar**; sin aseguramiento/acceso, mayor
  probabilidad y peor desenlace → **pérdida de productividad** (horas, ausentismo) →
  menor ingreso; en casos graves, mortalidad.
- **Entradas**: afiliación al SGSSS, ingreso, edad, ruralidad, desnutrición.
- **Forma funcional**: `p_enferma = base_edad · RR_sin_aseguramiento · RR_pobreza`;
  efecto `ingreso ·= (1 − pérdida_productividad)` mientras dura la enfermedad.
- **Parámetros + fuente**: cobertura aseguramiento ~**95-99%** y barreras de acceso
  (ECV); mortalidad infantil/materna por departamento (SISPRO/DANE); desnutrición
  (ENSIN). Efecto productividad: literatura de carga de enfermedad (a calibrar).
- **Calibración macro**: mortalidad y cobertura por departamento; barreras de acceso.

## M5 — Emergencia de la delincuencia (factores de riesgo)

> No existe una "probabilidad de delinquir" individual medida. Se modela con **factores
> de riesgo** y se **calibra para reproducir las tasas reales** de delito/homicidio.

- **Evento**: un agente puede **entrar a la delincuencia** (y, con la política, salir).
- **Entradas**: ser **nini** (ni estudia ni trabaja), bajo ingreso/pobreza, baja
  educación, desempleo juvenil, **presencia de crimen organizado/cultivos ilícitos** en
  el municipio, sexo y edad (hombres jóvenes mayor riesgo), redes de apoyo.
- **Forma funcional**: `p_delito = base · RR_nini · RR_pobreza · RR_baja_educ ·
  RR_crimen_local`, normalizada para que la **tasa municipal simulada = tasa real**.
- **Parámetros + fuente**: perfil nini (mujeres, menos educación, hogares de menor
  ingreso — LEE Javeriana 2022; determinantes nini por género, 2023); vínculo nini–
  crimen organizado y desempleo–delincuencia (literatura). **Anclas reales**: tasa de
  **homicidios** y **hurtos** por municipio (Medicina Legal / Policía Nacional),
  **cultivos ilícitos** (UNODC), percepción de seguridad (ECSC).
- **Calibración macro**: homicidios/100k y hurtos por municipio.

## M6 — Movilidad social intergeneracional

- **Evento**: los hijos heredan parcialmente las condiciones del hogar; la educación
  es el canal de movilidad.
- **Entradas**: educación/ingreso de los padres, acceso educativo, estrato.
- **Forma funcional**: correlación intergeneracional de ingreso/educación, atenuada por
  el logro educativo del hijo (M1+M2).
- **Parámetros + fuente**: persistencia intergeneracional alta en Colombia (ELCA,
  estudios de movilidad). **A calibrar** con la correlación padres-hijos.
- **Calibración macro**: movilidad social (ELCA), Gini intergeneracional.

## M7 — Demografía (nacer, envejecer, morir, formar hogar)

- **Evento**: cada año los agentes envejecen; ocurren nacimientos, muertes y formación/
  disolución de hogares y migración.
- **Entradas**: edad, sexo, fecundidad por edad, mortalidad por edad/departamento.
- **Forma funcional**: tablas de vida y tasas específicas por edad.
- **Parámetros + fuente**: fecundidad total ~**1,7-1,8**, esperanza de vida ~**77 años**,
  tasas por edad y departamento (DANE Estadísticas Vitales / proyecciones 2018-2035 —
  ya descargamos la población por municipio y año).
- **Calibración macro**: pirámide etaria y crecimiento poblacional por municipio (DANE).

---

## Cómo se conectan (red causal)

```
estrato/ingreso hogar ─┬─> M1 deserción ──> M2 educación→ingreso ──┐
                       │                                            ├─> ingreso/riqueza ─> economía (KWEM)
salud/aseguramiento ───┼─> M4 enfermar→productividad ──────────────┘
                       │
nini + pobreza + ──────┴─> M5 delincuencia ──> seguridad municipal ──> (afecta inversión, empleo)
crimen local
                            M3 empleo/informalidad  <──> ingreso, pensiones
                            M6 movilidad  (cierra el ciclo entre generaciones)
                            M7 demografía (motor de tiempo de todo lo anterior)
```

Cada **política** (escenario) interviene parámetros de estos mecanismos:
p.ej. *gratuidad superior* ↓ `p_desercion` (M1); *transferencias* ↑ ingreso hogar
(M1/M4/M5); *mano dura vs seguridad humana* cambia M5; *salud* cambia M4.

## Estado de los datos por mecanismo

| Mec | Parámetros con dato | Pendiente de descargar |
|-----|---------------------|------------------------|
| M1  | deserción por municipio (MEN ✅), por estrato (Javeriana) | deserción 5-16 por estrato fino |
| M2  | retorno ~10%/año (GEIH) | curva por nivel/edad/región |
| M3  | desempleo/informalidad nacional | por municipio (GEIH microdatos) |
| M4  | cobertura, mortalidad | efecto productividad de enfermedad |
| M5  | perfil nini, anclas | homicidios/hurtos por municipio (Med. Legal/Policía) |
| M6  | dirección (alta persistencia) | coeficiente (ELCA) |
| M7  | población por municipio/año (DANE ✅) | fecundidad/mortalidad por edad |

Valores numéricos y fuentes exactas: `data/reference/parametros.yaml`.
