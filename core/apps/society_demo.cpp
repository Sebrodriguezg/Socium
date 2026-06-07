// Demo: economía acoplada a la población real de Colombia.
//   socium_sociedad [--n N] [--sweeps S] [--rule dy|saving|yardsale]
//                   [--lambda L] [--f F] [--local 0|1] [--seed S] [--threads T]
#include "socium/population.hpp"
#include "socium/geography.hpp"
#include "socium/society.hpp"

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
    const std::int64_t n = std::atoll(arg(argc, argv, "--n", "5000000"));
    const std::int64_t sweeps = std::atoll(arg(argc, argv, "--sweeps", "200"));
    const std::uint64_t seed = std::strtoull(arg(argc, argv, "--seed", "42"), nullptr, 10);
    const bool local = std::atoi(arg(argc, argv, "--local", "1")) != 0;
    EconomyParams ep;
    ep.rule   = rule_from_string(arg(argc, argv, "--rule", "dy"));
    ep.lambda = std::atof(arg(argc, argv, "--lambda", "0.5"));
    ep.f      = std::atof(arg(argc, argv, "--f", "0.5"));
#ifdef _OPENMP
    const char* th = arg(argc, argv, "--threads", nullptr);
    if (th) omp_set_num_threads(std::atoi(th));
#endif

    Geography g = Geography::load_csv(arg(argc, argv, "--geo", "data/reference/divipola.csv"));
    g.cargar_pesos(arg(argc, argv, "--pob", "data/reference/poblacion_municipal.csv"));

    Population p = build_synthetic(n, seed);
    asignar_municipios(p, g, seed);

    std::cout << "== Socium · Economía sobre población real ==\n"
              << "  individuos : " << n << "   sweeps: " << sweeps << "\n"
              << "  regla      : " << rule_name(ep.rule)
              << "   emparejamiento: " << (local ? "por municipio" : "mean-field") << "\n"
              << "-- riqueza inicial (sintética) --\n"
              << "  Gini    : " << metrics::gini_riqueza(p) << "\n"
              << "  top 1%  : " << metrics::top_share_riqueza(p, 0.01) << "\n";

    const auto t0 = std::chrono::steady_clock::now();
    simular_economia(p, ep, sweeps, local, seed);
    const auto t1 = std::chrono::steady_clock::now();

    std::cout << "-- tras " << sweeps << " sweeps (" << std::chrono::duration<double>(t1 - t0).count()
              << " s) --\n"
              << "  Gini    : " << metrics::gini_riqueza(p) << "\n"
              << "  top 1%  : " << metrics::top_share_riqueza(p, 0.01) << "\n"
              << "  top 10% : " << metrics::top_share_riqueza(p, 0.10) << "\n";
    return 0;
}
