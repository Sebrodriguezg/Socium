#pragma once
// RNG por hilo, determinista y reproducible bajo OpenMP.
// Cada hilo obtiene una semilla derivada (splitmix64) de la semilla base + su id,
// de modo que una misma `seed` produce siempre el mismo resultado a igual nº de hilos.
#include <cstdint>
#include <random>
#ifdef _OPENMP
#include <omp.h>
#endif

namespace socium {

inline std::mt19937_64& thread_rng() {
    static thread_local std::mt19937_64 engine{std::random_device{}()};
    return engine;
}

// Mezcla splitmix64 para descorrelacionar semillas por hilo.
inline std::uint64_t splitmix64(std::uint64_t z) {
    z += 0x9E3779B97F4A7C15ULL;
    z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ULL;
    z = (z ^ (z >> 27)) * 0x94D049BB133111EBULL;
    return z ^ (z >> 31);
}

// Sembrar el RNG del hilo actual. Llamar dentro de una región paralela.
inline void seed_thread_rng(std::uint64_t base_seed) {
    std::uint64_t tid = 0;
#ifdef _OPENMP
    tid = static_cast<std::uint64_t>(omp_get_thread_num());
#endif
    thread_rng().seed(splitmix64(base_seed + 0x1000ULL * (tid + 1)));
}

// Uniforme en [0,1).
inline double uniform01() {
    return std::generate_canonical<double, 53>(thread_rng());
}

} // namespace socium
