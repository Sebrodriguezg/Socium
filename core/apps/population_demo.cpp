// Demo del substrato de población (individuos) de Colombia.
//   socium_poblacion [--n N] [--seed S] [--threads T]
// Construye una población sintética y reporta métricas agregadas (rol `A`).
#include "socium/population.hpp"

#include <chrono>
#include <cstdlib>
#include <cstring>
#include <iostream>
#ifdef _OPENMP
#include <omp.h>
#endif

using namespace socium;

static const char* arg(int argc, char** argv, const char* k, const char* d) {
    for (int i = 1; i + 1 < argc; ++i)
        if (std::strcmp(argv[i], k) == 0) return argv[i + 1];
    return d;
}

int main(int argc, char** argv) {
    const std::int64_t n = std::atoll(arg(argc, argv, "--n", "1000000"));
    const std::uint64_t seed = std::strtoull(arg(argc, argv, "--seed", "42"), nullptr, 10);
#ifdef _OPENMP
    const char* th = arg(argc, argv, "--threads", nullptr);
    if (th) omp_set_num_threads(std::atoi(th));
#endif

    std::cout << "== Socium · Población (individuos) ==\n"
              << "  N         : " << n << "\n"
              << "  bytes/ag  : " << Population::bytes_per_agent()
              << "  (~" << (n * Population::bytes_per_agent()) / (1024.0 * 1024.0) << " MB)\n";

    const auto t0 = std::chrono::steady_clock::now();
    Population p = build_synthetic(n, seed);
    const auto t1 = std::chrono::steady_clock::now();

    std::cout << "  build     : " << std::chrono::duration<double>(t1 - t0).count() << " s\n"
              << "-- métricas agregadas (placeholder, sin calibrar) --\n"
              << "  edad media        : " << metrics::mean_age(p) << " años\n"
              << "  % mujeres         : " << 100 * metrics::share_female(p) << " %\n"
              << "  desempleo         : " << 100 * metrics::unemployment_rate(p) << " %\n"
              << "  informalidad      : " << 100 * metrics::informality_rate(p) << " %\n"
              << "  ingreso lab. medio: " << metrics::mean_labor_income(p) << " COP/mes\n"
              << "  Gini ingreso      : " << metrics::gini_income(p) << "\n";
    return 0;
}
