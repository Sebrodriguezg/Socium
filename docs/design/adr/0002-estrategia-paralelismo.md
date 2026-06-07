# ADR 0002 — Estrategia de paralelismo y hardware objetivo

- **Fecha**: 2026-06-06
- **Estado**: aceptado

## Contexto
El diagnóstico (`hpc/probe_env.sh`) reveló que el hardware de pruebas pesadas es
**una única estación de trabajo**, no un clúster multi-nodo:
Threadripper 3990X (64c/128t), 251 GB RAM, 1× RTX 2080 Ti, OpenMPI 4.1.2,
g++ 11.4 con C++20+OpenMP funcionando, **sin scheduler ni sistema de módulos**.

## Decisión
- Se confirma **C++20 como lenguaje del núcleo** (validado: compila y usa 128 hilos).
- **OpenMP** es el mecanismo de paralelismo **principal** (memoria compartida).
- **MPI** queda **secundario**: se mantiene la abstracción en `core/src/parallel`,
  pero no se prioriza mientras el objetivo sea un solo nodo.
- **GPU (CUDA, sm_75)** reservada para *offload* futuro de kernels intensivos.
- Compilación dirigida a Zen2: `-march=znver2 -mtune=znver2`.
- Toolchain faltante (cmake, ninja, git-lfs, pybind11, etc.) se provee vía **conda
  sin sudo** (`hpc/environment.yml`).
- Sin scheduler: corridas largas con `tmux`/`nohup`.

## Consecuencias
- Diseño de datos *cache-friendly* y *thread-scalable* es la prioridad de rendimiento.
- El código debe permanecer agnóstico para escalar a MPI si aparece multi-nodo,
  sin reescritura del modelo.
