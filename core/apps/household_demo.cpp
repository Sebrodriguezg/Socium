// Demo de formación de hogares.
//   socium_hogares [--n N] [--seed S] [--threads T]
// Construye población, la localiza en municipios reales y forma hogares; reporta
// métricas de hogar (rol `A` / `Hg`).
#include "socium/population.hpp"
#include "socium/geography.hpp"
#include "socium/household.hpp"

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
    const std::uint64_t seed = std::strtoull(arg(argc, argv, "--seed", "42"), nullptr, 10);
#ifdef _OPENMP
    const char* th = arg(argc, argv, "--threads", nullptr);
    if (th) omp_set_num_threads(std::atoi(th));
#endif

    Geography g = Geography::load_csv(arg(argc, argv, "--geo", "data/reference/divipola.csv"));
    g.cargar_pesos(arg(argc, argv, "--pob", "data/reference/municipios.csv"));

    Population p = build_synthetic(n, seed);
    asignar_municipios(p, g, seed);

    const auto t0 = std::chrono::steady_clock::now();
    Households h = form_households(p, g, seed);
    const auto t1 = std::chrono::steady_clock::now();

    std::cout << "== Socium · Hogares ==\n"
              << "  individuos     : " << n << "\n"
              << "  hogares        : " << h.size() << "\n"
              << "  form. hogares  : " << std::chrono::duration<double>(t1 - t0).count() << " s\n"
              << "-- métricas de hogar (placeholder) --\n"
              << "  tamaño medio   : " << metrics::mean_household_size(h) << " personas\n"
              << "  % unipersonal  : " << 100 * metrics::share_tipo(h, TipoHogar::Unipersonal) << " %\n"
              << "  % nuclear      : " << 100 * metrics::share_tipo(h, TipoHogar::Nuclear) << " %\n"
              << "  % extenso      : " << 100 * metrics::share_tipo(h, TipoHogar::Extenso) << " %\n"
              << "  % compuesto    : " << 100 * metrics::share_tipo(h, TipoHogar::Compuesto) << " %\n"
              << "  estrato medio  : " << metrics::mean_estrato(h) << "\n"
              << "  Gini ing. hogar: " << metrics::gini_household_income(h) << "\n";
    return 0;
}
