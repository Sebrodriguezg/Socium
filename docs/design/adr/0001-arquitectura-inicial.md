# ADR 0001 — Arquitectura inicial

- **Fecha**: 2026-06-06
- **Estado**: aceptado

## Contexto
Socium busca simular una sociedad completa (economía + social + políticas) a gran
escala, con vistas a publicación abierta (Git), página web y API.

## Decisión
- **Monorepo.**
- Núcleo de simulación en **C++** (HPC: OpenMP/MPI), en `core/`.
- **Python** para análisis, visualización y calibración, en `python/`.
- Puente **pybind11** en `bindings/`.
- Bibliografía versionada en `articles/` con **Git LFS** para los PDFs.
- Especificación del modelo vía **protocolo ODD** en `docs/model-spec/`.

## Consecuencias
- Máximo rendimiento y escalabilidad en el motor; ergonomía de Python para ciencia.
- Coste: mantener el puente C++/Python y dos toolchains de build.

> Las decisiones de arquitectura futuras se registran como nuevos ADR numerados aquí.
