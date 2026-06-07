# Especificación del modelo — Protocolo ODD

El modelo se documenta siguiendo el protocolo **ODD** (Overview, Design concepts,
Details), el estándar para describir modelos basados en agentes de forma reproducible
(Grimm et al., 2006/2020). Esta es la fuente de verdad de *qué* hace el modelo;
el código en `core/` es *cómo* lo hace.

## 1. Overview

### 1.1 Propósito
*(¿Qué pregunta responde el modelo? ¿Qué políticas se quieren evaluar?)*

### 1.2 Entidades, variables de estado y escalas
- **Agentes**: individuos / hogares / firmas / instituciones.
- **Variables de estado**: edad, educación, ingreso, riqueza, empleo, red social, ...
- **Escala espacial y temporal**: (p.ej. 1 tick = 1 mes; N agentes).

### 1.3 Visión general del proceso y calendario (scheduling)
*(Orden de los submodelos en cada paso de tiempo.)*

## 2. Design concepts
*(Emergencia, adaptación, objetivos, aprendizaje, interacción, estocasticidad,
colectivos, observación.)*

## 3. Details

### 3.1 Inicialización
### 3.2 Datos de entrada
### 3.3 Submodelos
- **Económico** (`core/src/economy`)
- **Social / redes** (`core/src/social`)
- **Políticas** (`core/src/policies`): educación, empleo, salud, ...
- **Demografía / emparejamiento**
