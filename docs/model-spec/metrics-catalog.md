# Catálogo de métricas — Socium · Colombia

Documento maestro de los **indicadores que definen el estado del país** en el modelo
de agentes. Es un documento **vivo**: aquí se define *todo lo que queremos medir* del
"Colombia de computador".

Cada métrica se etiqueta con:

- **Nivel**: `Ag` agente (individuo) · `Hg` hogar · `Mun` municipio · `Dep` departamento ·
  `Nac` nacional.
- **Rol** en el modelo:
  - `E` = variable de **estado** del agente/hogar (lo que el agente *es*).
  - `A` = indicador **agregado/emergente** (output que el modelo *produce*).
  - `P` = **palanca de política** (parámetro que podemos intervenir).
  - `C` = objetivo de **calibración** (lo ajustamos contra el dato real).
- **Fuente (CO)**: de dónde sale el dato real para calibrar/validar.

> Niveles territoriales de Colombia para el modelo: **Nación → 32 departamentos +
> Bogotá D.C. → ~1.103 municipios → urbano/rural**, con las regiones DANE (Caribe,
> Pacífica, Andina, Orinoquía, Amazonía) y categorías de ruralidad del DNP.

Fuentes principales: **DANE** (Censo 2018, GEIH, ECV, ENPH, ECSC, Cuentas Nacionales),
**Banco de la República**, **DNP / TerriData**, **Medicina Legal**, **SISBÉN IV (DNP)**,
**MinEducación (SIMAT, SNIES, Saber/ICFES)**, **MinSalud (SISPRO, ADRES, ENSIN)**,
**Migración Colombia**, **IDEAM**, **UARIV (víctimas)**, **Registraduría**, **DIAN**,
**PNUD/CEPAL/Banco Mundial/OCDE**, **LAPOP/Barómetro de las Américas**.

---

## 0. Demografía y hogares

| Métrica | Definición / unidad | Nivel | Rol | Fuente (CO) |
|---|---|---|---|---|
| Edad | años | Ag | E,C | Censo, GEIH |
| Sexo / género | categórica | Ag | E,C | Censo |
| Pertenencia étnica | indígena/NARP/Rrom/ninguna | Ag | E,C | Censo |
| Tamaño del hogar | nº de personas | Hg | E,C | ECV, Censo |
| Tipo de hogar | unipersonal/nuclear/extenso… | Hg | E | ECV |
| Estado civil / unión | categórica | Ag | E | Censo |
| Tasa de natalidad | nacimientos / 1.000 hab·año | Nac/Dep | A,C | DANE Estadísticas Vitales |
| Tasa de fecundidad total | hijos por mujer | Dep | A,C | DANE/ENDS |
| Tasa de mortalidad | defunciones / 1.000 hab·año | Dep | A,C | DANE EEVV |
| Esperanza de vida al nacer | años | Dep/Nac | A,C | DANE |
| Estructura etaria / pirámide | % por grupos de edad | Dep | A,C | Censo, proyecciones |
| Razón de dependencia | (menores+mayores)/PEA | Dep | A | DANE |
| Densidad poblacional | hab/km² | Mun | E,C | Censo |
| Tasa de urbanización | % población en cabecera | Mun/Nac | A,C | Censo |

## 1. Economía macro

| Métrica | Definición / unidad | Nivel | Rol | Fuente (CO) |
|---|---|---|---|---|
| PIB | COP constantes y per cápita | Nac/Dep | A,C | DANE Cuentas Nacionales |
| Crecimiento del PIB | % anual | Nac/Dep | A,C | DANE |
| PIB por sectores | agro/industria/servicios/minero | Dep | A,C | DANE |
| Inflación (IPC) | % anual | Nac | A,P,C | DANE / BanRep |
| Tasa de interés de política | % | Nac | P | BanRep |
| Tasa de cambio | COP/USD | Nac | A,C | BanRep |
| Productividad laboral | producto / ocupado | Dep | A,C | DANE |
| Inversión (FBKF) | % del PIB | Nac | A,P | DANE |
| Déficit/superávit fiscal | % del PIB | Nac | A,P | MinHacienda |
| Deuda pública | % del PIB | Nac | A,P | MinHacienda |
| Recaudo tributario | COP, % PIB | Nac/Mun | A,P,C | DIAN / DNP |
| Gasto público social | COP por habitante, por sector | Nac/Dep | P,C | DNP |

## 2. Trabajo y mercado laboral

| Métrica | Definición / unidad | Nivel | Rol | Fuente (CO) |
|---|---|---|---|---|
| Situación laboral | ocupado/desocupado/inactivo | Ag | E,C | GEIH |
| Tasa de desempleo | % de la PEA | Mun/Dep/Nac | A,C | GEIH |
| Tasa de ocupación | % en edad de trabajar | Dep | A,C | GEIH |
| Tasa global de participación | PEA / PET | Dep | A,C | GEIH |
| **Informalidad laboral** | % ocupados informales | Mun/Dep | A,P,C | GEIH (clave en CO) |
| Subempleo | % subempleados | Dep | A,C | GEIH |
| Sector económico del empleo | rama CIIU | Ag | E | GEIH |
| Posición ocupacional | asalariado/cuenta propia/empleador | Ag | E | GEIH |
| Salario / ingreso laboral | COP/mes | Ag | E,C | GEIH |
| Salario mínimo | COP/mes | Nac | P | Gobierno |
| Cobertura pensional | % cotizantes/afiliados | Ag/Dep | E,A,P | PILA/ADRES |
| Horas trabajadas | horas/semana | Ag | E | GEIH |
| Brecha salarial de género | % | Dep/Nac | A,C | GEIH |
| Trabajo infantil | % menores 5-17 trabajando | Dep | A,C | GEIH módulo |

## 3. Ingreso, riqueza, desigualdad y pobreza

| Métrica | Definición / unidad | Nivel | Rol | Fuente (CO) |
|---|---|---|---|---|
| Ingreso del hogar | COP/mes (per cápita) | Hg | E,C | GEIH/ENPH |
| Riqueza / patrimonio | COP (stock) | Ag/Hg | E,A | ENPH (proxy) |
| **Coeficiente de Gini** | [0,1] ingreso/riqueza | Mun/Dep/Nac | A,C | DANE |
| Exponente de Pareto (cola alta) | adimensional | Nac | A | derivado |
| Participación del top 1% / 10% | % del ingreso/riqueza | Nac | A,C | DIAN/DANE |
| **Pobreza monetaria** | % bajo línea de pobreza | Mun/Dep/Nac | A,P,C | DANE |
| Pobreza extrema | % bajo línea de indigencia | Dep | A,C | DANE |
| **Pobreza multidimensional (IPM)** | % hogares (15 privaciones) | Mun/Dep | A,P,C | DANE (oficial CO) |
| NBI | % con necesidades básicas insatisfechas | Mun | A,C | Censo |
| Clasificación SISBÉN IV | grupos A/B/C/D | Hg | E,P | DNP |
| Movilidad social intergeneracional | correlación ingreso padres-hijos | Nac | A | estudios/ELCA |
| Transferencias recibidas | COP (subsidios) | Hg | E,P | Prosperidad Social |

## 4. Educación

| Métrica | Definición / unidad | Nivel | Rol | Fuente (CO) |
|---|---|---|---|---|
| Años de escolaridad | años | Ag | E,C | GEIH/Censo |
| Nivel educativo alcanzado | ninguno…posgrado | Ag | E,C | Censo |
| Asistencia escolar | sí/no | Ag | E,C | SIMAT |
| Tasa de matrícula (neta/bruta) | % por nivel | Mun/Dep | A,P,C | SIMAT |
| Tasa de deserción escolar | % | Mun/Dep | A,P,C | MinEducación |
| Cobertura educación superior | % | Dep | A,P,C | SNIES |
| Tasa de alfabetización | % ≥15 años | Dep | A,C | Censo |
| Resultados Saber 11 / PISA | puntaje | Mun/Nac | A,C | ICFES/OCDE |
| Razón alumno/docente | nº | Mun | E,C | MinEducación |
| Gasto público en educación | % PIB / por alumno | Nac | P | DNP |
| Calidad docente | escalafón/formación | Mun | E | MinEducación |

## 5. Salud

| Métrica | Definición / unidad | Nivel | Rol | Fuente (CO) |
|---|---|---|---|---|
| Afiliación al SGSSS | contributivo/subsidiado/no | Ag | E,P,C | ADRES/SISPRO |
| Cobertura de aseguramiento | % afiliados | Dep | A,P,C | MinSalud |
| Mortalidad infantil | / 1.000 nacidos vivos | Dep | A,C | DANE/SISPRO |
| Mortalidad materna | / 100.000 nacidos | Dep | A,C | SISPRO |
| Desnutrición infantil | % crónica/aguda | Dep | A,C | ENSIN |
| Prevalencia enfermedades crónicas | % | Dep | A | SISPRO |
| Salud mental / suicidio | tasa / 100.000 | Dep | A,C | Medicina Legal |
| Embarazo adolescente | % nacidos de madres <19 | Dep | A,C | DANE |
| Esperanza de vida saludable | años | Nac | A | OMS |
| Médicos / camas por habitante | nº / 1.000 hab | Dep | E,C | MinSalud |
| Gasto en salud | % PIB / per cápita | Nac | P | DNP |
| Acceso/barreras a servicios | % con barrera | Dep | A,C | ECV |

## 6. Vivienda y servicios públicos

| Métrica | Definición / unidad | Nivel | Rol | Fuente (CO) |
|---|---|---|---|---|
| Tenencia de vivienda | propia/arriendo/otra | Hg | E,C | ECV |
| Déficit habitacional | % (cuantitativo/cualitativo) | Mun | A,P,C | DANE |
| Hacinamiento | personas/cuarto | Hg | E,C | ECV |
| Acceso a acueducto | % con cobertura | Mun | A,P,C | DANE/SUI |
| Acceso a alcantarillado | % | Mun | A,P,C | SUI |
| Acceso a energía eléctrica | % | Mun | A,C | SUI |
| Acceso a gas/recolección basuras | % | Mun | A,C | SUI |
| Estrato socioeconómico | 1–6 | Hg | E,P | catastro |
| Material de paredes/pisos | categórica | Hg | E | Censo |

## 7. Seguridad, violencia y conflicto armado

| Métrica | Definición / unidad | Nivel | Rol | Fuente (CO) |
|---|---|---|---|---|
| Tasa de homicidios | / 100.000 hab | Mun/Dep | A,C | Medicina Legal/Policía |
| Hurtos / delitos contra patrimonio | tasa | Mun | A,C | Policía Nacional |
| Violencia intrafamiliar | tasa | Mun | A,C | Medicina Legal |
| Violencia de género / feminicidios | tasa | Dep | A,C | Medicina Legal |
| Víctimas del conflicto armado | nº registrados | Mun | E,A,C | UARIV/RUV |
| Desplazamiento forzado | personas/año | Mun | A,C | UARIV |
| Presencia de grupos armados | índice/categórica | Mun | E,P | FIP/Defensoría |
| Cultivos ilícitos | hectáreas de coca | Mun | A,P,C | UNODC/SIMCI |
| Minas antipersona / secuestro/extorsión | nº | Mun | A,C | entidades |
| Percepción de seguridad | % se siente seguro | Mun | A,C | ECSC (DANE) |

## 8. Instituciones, justicia y gobernanza

| Métrica | Definición / unidad | Nivel | Rol | Fuente (CO) |
|---|---|---|---|---|
| Índice de corrupción/transparencia | puntaje | Dep/Nac | A,P,C | Transparencia por Colombia |
| Capacidad/desempeño fiscal municipal | índice | Mun | E,C | DNP |
| Eficiencia del gasto público | índice | Mun | A | DNP |
| Acceso a la justicia / impunidad | % casos resueltos | Dep | A,C | Fiscalía/Rama Judicial |
| Confianza en instituciones | % confía | Nac | A,C | LAPOP |
| Estado de derecho | índice | Nac | A,C | WJP |
| Cobertura del Estado/presencia institucional | índice | Mun | E,P | DNP |
| Tasa de tramitología / registro empresarial | nº/tiempo | Mun | A | Confecámaras |

## 9. Capital social, cultura y confianza

| Métrica | Definición / unidad | Nivel | Rol | Fuente (CO) |
|---|---|---|---|---|
| Confianza interpersonal | % confía en otros | Ag/Nac | E,A,C | LAPOP/ECC |
| Participación en organizaciones | % pertenece | Ag | E,C | ECC (DANE) |
| Capital social (índice) | compuesto | Dep | A | Barómetro |
| Religión / religiosidad | categórica/% | Ag | E | encuestas |
| Tolerancia / discriminación | índice | Nac | A,C | LAPOP |
| Redes de apoyo del hogar | sí/no | Hg | E | ECV |

## 10. Política, opinión y participación

| Métrica | Definición / unidad | Nivel | Rol | Fuente (CO) |
|---|---|---|---|---|
| Opinión/orientación política | espectro/categórica | Ag | E,A | encuestas |
| Intención de voto / preferencia | categórica | Ag | E,A | encuestas |
| Abstención electoral | % | Mun/Nac | A,C | Registraduría |
| Polarización política | índice de dispersión | Nac | A | derivado |
| Aprobación del gobierno | % | Nac | A,C | encuestas |
| Protesta social | nº eventos / participación | Mun | A | observatorios |
| Confianza en la democracia | % apoyo | Nac | A,C | LAPOP |

## 11. Medio ambiente, clima y recursos

| Métrica | Definición / unidad | Nivel | Rol | Fuente (CO) |
|---|---|---|---|---|
| Emisiones de CO₂eq | toneladas | Nac/Dep | A,P,C | IDEAM |
| Deforestación | hectáreas/año | Mun/Dep | A,P,C | IDEAM/SMByC |
| Calidad del aire (PM2.5) | µg/m³ | Mun | A,C | SISAIRE/IDEAM |
| Acceso a agua potable segura | % | Mun | A,C | SUI |
| Cobertura forestal / áreas protegidas | % territorio | Dep | A | IDEAM/PNN |
| Vulnerabilidad/riesgo climático | índice | Mun | E,C | IDEAM/UNGRD |
| Desastres naturales | nº eventos/afectados | Mun | A | UNGRD |
| Huella hídrica / estrés hídrico | índice | Dep | A | IDEAM |
| Biodiversidad | índice | Dep | A | IAvH |

## 12. Infraestructura, movilidad y conectividad digital

| Métrica | Definición / unidad | Nivel | Rol | Fuente (CO) |
|---|---|---|---|---|
| Red vial / estado | km, % pavimentado | Dep | E,P,C | INVÍAS/ANI |
| Tiempo de viaje a cabecera/servicios | minutos | Mun | A,C | DNP/TerriData |
| Penetración de internet | % hogares con acceso | Mun/Dep | A,P,C | MinTIC |
| Penetración móvil / smartphones | % | Dep | A,C | MinTIC |
| Velocidad de internet | Mbps | Dep | A | MinTIC |
| Acceso a energía (cobertura) | % | Mun | A,C | UPME |
| Bancarización / inclusión financiera | % con producto financiero | Dep | A,P,C | Banca de las Oportunidades |
| Uso de gobierno digital | % trámites en línea | Nac | A | MinTIC |

## 13. Migración y movilidad poblacional

| Métrica | Definición / unidad | Nivel | Rol | Fuente (CO) |
|---|---|---|---|---|
| Migración interna | flujos entre municipios | Mun | A,C | Censo/GEIH |
| Inmigración (venezolana) | nº/stock por municipio | Mun/Dep | E,A,C | Migración Colombia |
| Emigración internacional | nº/año | Nac | A,C | Migración Colombia |
| Remesas | USD recibidos | Dep/Nac | A,C | BanRep |
| Desplazamiento intraurbano | nº | Mun | A | UARIV |
| Estatus migratorio (PPT) | regular/irregular | Ag | E,P | Migración |

## 14. Género y grupos poblacionales

| Métrica | Definición / unidad | Nivel | Rol | Fuente (CO) |
|---|---|---|---|---|
| Brecha de género (participación laboral) | pp | Dep | A,C | GEIH |
| Brecha de ingresos por género | % | Dep | A,C | GEIH |
| Uso del tiempo / trabajo no remunerado | horas | Ag | E,C | ENUT (DANE) |
| Mujeres en cargos de poder | % | Nac | A,C | ONU Mujeres |
| Indicadores por grupo étnico | varios desagregados | Dep | A,C | Censo |
| Brechas urbano-rural | razón rural/urbano de cada métrica | Mun | A,C | derivado |

## 15. Bienestar subjetivo

| Métrica | Definición / unidad | Nivel | Rol | Fuente (CO) |
|---|---|---|---|---|
| Satisfacción con la vida | escala 0–10 | Ag | E,A,C | ECSC/encuestas |
| Felicidad percibida | escala | Ag | A,C | World Happiness/encuestas |
| Percepción de situación económica | mejor/igual/peor | Hg | A,C | encuestas |
| Expectativas a futuro | índice | Ag | E,A | encuestas |

---

## Índices compuestos (resumen del estado del país)

Indicadores "sombrilla" que el modelo debe reproducir como síntesis:

- **IDH** (PNUD) — salud + educación + ingreso.
- **IPM** (DANE) — pobreza multidimensional, estándar oficial colombiano.
- **Gini** — desigualdad.
- **Índice de Capacidades / Desempeño municipal** (DNP).
- **Índice de Competitividad Departamental** (CPC/Universidad del Rosario).
- **Índice de Paz / conflicto**.
- **Doing Business / clima de negocios**.

---

## Cómo se usan estas métricas en el modelo

1. **Inicialización (`E`,`C`)**: poblar los agentes/hogares para que las distribuciones
   iniciales por municipio reproduzcan el dato real (Censo 2018 + GEIH + IPM…).
2. **Dinámica**: las reglas de interacción actualizan las variables de estado `E`.
3. **Agregación (`A`)**: cada paso de tiempo el modelo calcula los indicadores agregados
   por municipio/departamento/nación → es "medir el país en el computador".
4. **Validación (`C`)**: comparar los agregados simulados vs. las series reales.
5. **Políticas (`P`)**: intervenir las palancas y medir el efecto en todos los `A`.

> **Pendiente de priorización**: este catálogo es deliberadamente amplio. El siguiente
> paso es elegir el **subconjunto del MVP** (las métricas con mejor dato disponible y
> mayor poder explicativo) y formalizarlas en `metrics.yaml` (esquema legible por el
> código, con tipo, unidad, nivel, fuente y enlace a la variable del agente).
