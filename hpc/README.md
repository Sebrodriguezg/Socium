# HPC — Entorno de ejecución

## El nodo (descubierto con `probe_env.sh`, 2026-06-06)

Es **una sola estación de trabajo** (no un clúster multi-nodo), pero muy potente:

| Recurso     | Detalle |
|-------------|---------|
| CPU         | AMD Threadripper 3990X — **64 núcleos / 128 hilos**, 1 socket, Zen2 |
| SIMD        | AVX2 + FMA (**sin AVX-512**) → compilar con `-march=znver2` |
| RAM         | **251 GB** |
| GPU         | 1× NVIDIA RTX 2080 Ti, 11 GB, compute 7.5, CUDA disponible (`nvcc`) |
| Compilador  | g++ 11.4 (C++20 + OpenMP ✔, 128 hilos) |
| MPI         | OpenMPI 4.1.2 |
| Scheduler   | **Ninguno** — nodo interactivo, se ejecuta directo (sin SLURM/colas) |
| Python      | miniconda, Python 3.13 |

## Implicaciones de arquitectura

- **OpenMP es el motor de paralelismo principal** (memoria compartida, 128 hilos).
- **MPI queda secundario** (preparado en `core/src/parallel`, útil solo si aparece
  multi-nodo). No es prioridad mientras sea un único nodo.
- **GPU (RTX 2080 Ti)** reservada para *offload* futuro de kernels pesados (CUDA, sm_75).
- Sin scheduler: las corridas largas conviene lanzarlas con `tmux`/`nohup`.

## Puesta a punto (sin sudo)

Todo lo que falta (cmake, ninja, ccache, git-lfs, pybind11, pandas…) se instala
con conda, sin permisos de admin:

```bash
conda env create -f hpc/environment.yml
conda activate socium
git lfs install            # ahora sí, una vez instalado por conda
```

### Compilar el núcleo (cuando haya fuentes)

```bash
conda activate socium
cmake -S core -B build -G Ninja \
      -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_CXX_FLAGS="-march=znver2 -mtune=znver2"
cmake --build build -j$(nproc)
```

### Lanzar una corrida larga sin colas

```bash
OMP_NUM_THREADS=128 nohup ./build/socium experiments/configs/<escenario>.yaml \
    > experiments/runs/<escenario>.log 2>&1 &
```

> **Nota MPI:** `mpi4py` de conda-forge trae su propio runtime (MPICH), que puede no
> coincidir con el OpenMPI 4.1.2 del sistema. Como aquí MPI no es prioritario, no hay
> conflicto en la práctica; si más adelante usamos MPI en serio, fijaremos una sola
> implementación.
