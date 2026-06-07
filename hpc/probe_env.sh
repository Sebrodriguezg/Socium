#!/usr/bin/env bash
# =====================================================================
#  Socium — Diagnóstico de entorno de clúster (sin sudo, no instala nada)
#  Uso:   bash hpc/probe_env.sh
#  Salida legible + se guarda en  probe_env_<host>_<fecha>.txt
# =====================================================================
set -u

OUT="probe_env_$(hostname -s 2>/dev/null || echo host)_$(date +%Y%m%d_%H%M).txt"
exec > >(tee "$OUT") 2>&1

line(){ printf '\n========== %s ==========\n' "$1"; }
have(){ command -v "$1" >/dev/null 2>&1; }
ver(){ # ver <cmd> <args...>
  local c="$1"; shift
  if have "$c"; then
    printf '  [OK ] %-12s -> %s\n' "$c" "$(command -v "$c")"
    "$c" "$@" 2>&1 | head -n 2 | sed 's/^/        /'
  else
    printf '  [ -- ] %-12s no encontrado\n' "$c"
  fi
}

line "SISTEMA"
uname -a
[ -r /etc/os-release ] && grep -E 'PRETTY_NAME' /etc/os-release
echo "  Hostname: $(hostname -f 2>/dev/null || hostname)"

line "CPU / MEMORIA"
if have lscpu; then lscpu | grep -E 'Model name|^CPU\(s\)|Socket|Core|Thread|MHz|Flags' | sed 's/^/  /'; fi
echo "  --- flags SIMD relevantes ---"
grep -m1 -oE '(avx512[a-z]*|avx2|avx|sse4_2|fma)' /proc/cpuinfo 2>/dev/null | sort -u | sed 's/^/  /'
if have free; then free -h | sed 's/^/  /'; fi
echo "  Nodos lógicos visibles: $(nproc 2>/dev/null || echo '?')"

line "COMPILADORES C / C++ / Fortran"
ver gcc --version
ver g++ --version
ver clang --version
ver icc --version
ver icpx --version       # Intel oneAPI
ver nvc++ --version      # NVIDIA HPC SDK
ver gfortran --version
ver ifort --version

line "SOPORTE C++20 / OpenMP (prueba real de compilación)"
TMPD="$(mktemp -d)"
cat > "$TMPD/t.cpp" <<'EOF'
#include <version>
#include <omp.h>
#include <iostream>
int main(){ int n=0;
  #pragma omp parallel
  { ; n=omp_get_num_threads(); }
  std::cout << "  C++ __cplusplus="<<__cplusplus<<"  OMP_threads="<<n<<"\n";
}
EOF
if have g++; then
  if g++ -std=c++20 -fopenmp "$TMPD/t.cpp" -o "$TMPD/t" 2>"$TMPD/err"; then
    echo "  [OK ] g++ compila C++20 + OpenMP"; "$TMPD/t"
  else
    echo "  [WARN] g++ no compiló C++20+OpenMP:"; sed 's/^/        /' "$TMPD/err" | head -n 5
  fi
fi
rm -rf "$TMPD"

line "MPI"
ver mpicc --version
ver mpicxx --version
ver mpirun --version
ver mpiexec --version
have mpirun && echo "  Implementación:" && mpirun --version 2>&1 | head -n1 | sed 's/^/    /'

line "BUILD / TOOLING"
ver cmake --version
ver make --version
ver ninja --version
ver ccache --version
ver git --version
ver git-lfs version
ver pkg-config --version

line "PYTHON"
ver python3 --version
ver python --version
ver pip3 --version
ver conda --version
ver mamba --version
ver micromamba --version
if have python3; then
  echo "  --- paquetes científicos en python3 base ---"
  python3 - <<'PY' 2>/dev/null | sed 's/^/  /'
for m in ("numpy","scipy","pandas","mpi4py","numba","cython","h5py","networkx","matplotlib","pybind11"):
    try:
        mod=__import__(m); print(f"{m:10s} {getattr(mod,'__version__','?')}")
    except Exception: print(f"{m:10s} --")
PY
fi

line "OTROS LENGUAJES (alternativas posibles)"
ver julia --version
ver rustc --version
ver go version
ver R --version
ver java -version

line "GPU / ACELERADORES"
if have nvidia-smi; then nvidia-smi --query-gpu=name,memory.total,driver_version,compute_cap --format=csv 2>&1 | sed 's/^/  /'; else echo "  nvidia-smi no encontrado (sin GPU NVIDIA visible en este nodo)"; fi
ver nvcc --version
have rocminfo && echo "  ROCm/AMD detectado"

line "GESTOR DE COLAS (scheduler)"
if have sbatch; then echo "  [OK ] SLURM"; ver sinfo --version
  echo "  --- particiones (sinfo) ---"; sinfo -o '%P %a %l %D %c %m %G' 2>/dev/null | head -n 20 | sed 's/^/    /'
elif have qsub; then echo "  [OK ] PBS/Torque o SGE (qsub presente)"; ver qstat -V
elif have bsub; then echo "  [OK ] LSF (bsub presente)"
else echo "  No se detectó scheduler conocido (¿nodo de login interactivo?)"; fi

line "MODULES (software pre-instalado y cargable sin sudo)"
if have module || [ -n "${LMOD_CMD:-}" ] || [ -n "${MODULESHOME:-}" ]; then
  echo "  Sistema de módulos detectado."
  echo "  --- module avail (resumen) ---"
  ( module avail 2>&1 || true ) | sed 's/^/    /' | head -n 120
  echo "  ... (corta a 120 líneas; ejecuta 'module avail' completo y busca: gcc, openmpi, mpich, intel, cmake, python, anaconda, cuda, julia)"
else
  echo "  No hay 'module'. Revisa también: /opt, /usr/local, spack, easybuild."
  have spack && echo "  spack disponible: 'spack find'"
fi

line "RESUMEN"
echo "  Archivo guardado en: $OUT"
echo "  Comparte ese archivo y decidimos toolchain (C++/MPI vs alternativas)."
