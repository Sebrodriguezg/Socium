# Socium

**Un simulador computacional de gran escala de una sociedad completa.**

Socium es un proyecto de **sociofísica + HPC** cuyo objetivo es construir el modelo
más completo posible de cómo funciona una sociedad en sus dimensiones **económica**,
**social** y **de políticas públicas** (educación, empleo, salud, oportunidades,
demografía), acopladas en un mismo motor de simulación basado en agentes (ABM) capaz
de escalar a millones de agentes sobre infraestructura de cómputo de alto rendimiento.

La meta es que cada política sea un **módulo intervenible** que se pueda activar,
desactivar y parametrizar para realizar experimentos contrafactuales reproducibles
("¿qué pasa si...?").

## Arquitectura (monorepo)

| Carpeta        | Propósito |
|----------------|-----------|
| `core/`        | Motor de simulación ABM en **C++** (HPC, MPI/OpenMP). |
| `bindings/`    | Puente C++ ↔ Python (pybind11). |
| `python/`      | Paquete Python de análisis, visualización y calibración. |
| `experiments/` | Configuraciones de escenarios, corridas y notebooks. |
| `data/`        | Datos empíricos para calibración/validación. |
| `hpc/`         | Scripts de clúster (SLURM), entornos y contenedores. |
| `web/`         | Página web (frontend) para divulgación de resultados. |
| `api/`         | API (backend) para consultar y lanzar escenarios. |
| `docs/`        | Teoría, especificación del modelo (ODD), decisiones de diseño. |
| `articles/`    | Bibliografía: `.bib`, PDFs (Git LFS), notas y por tema. |
| `results/`     | Figuras y tablas curadas para publicaciones. |
| `scripts/`     | Utilidades de línea de comandos. |

## Estado

🚧 Fase inicial: recopilación bibliográfica y diseño de arquitectura.

## Licencia

Por definir (ver `LICENSE`).
