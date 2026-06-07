// Demo del substrato de población (individuos) de Colombia.
//   socium_poblacion [--n N] [--seed S] [--threads T]
// Construye una población sintética y reporta métricas agregadas (rol `A`).
#include "socium/population.hpp"
#include "socium/geography.hpp"

#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <vector>
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

    // Distribución espacial real (ponderada por población) si hay geografía.
    const char* geo_path = arg(argc, argv, "--geo", "data/reference/divipola.csv");
    const char* pob_path = arg(argc, argv, "--pob", "data/reference/municipios.csv");
    try {
        Geography g = Geography::load_csv(geo_path);
        const std::int64_t con_peso = g.cargar_pesos(pob_path);
        asignar_municipios(p, g, seed);

        std::vector<std::int64_t> por_mpio(g.n_municipios(), 0);
        for (std::int64_t i = 0; i < p.size(); ++i) ++por_mpio[p.municipio_id[i]];

        std::vector<std::int64_t> ord(g.n_municipios());
        for (std::int64_t m = 0; m < g.n_municipios(); ++m) ord[m] = m;
        std::partial_sort(ord.begin(), ord.begin() + 8, ord.end(),
                          [&](std::int64_t a, std::int64_t b) { return por_mpio[a] > por_mpio[b]; });

        std::cout << "-- distribución espacial (" << con_peso << " municipios con peso) --\n"
                  << "  top 8 municipios por nº de agentes:\n";
        for (int k = 0; k < 8; ++k) {
            const std::int64_t m = ord[k];
            std::cout << "    " << g.mpio_nombre[m] << " (" << g.dpto_nombre[g.mpio_dpto[m]]
                      << "): " << por_mpio[m] << "  ("
                      << 100.0 * por_mpio[m] / p.size() << " %)\n";
        }
    } catch (const std::exception& e) {
        std::cout << "(sin distribución espacial: " << e.what() << ")\n";
    }
    return 0;
}
