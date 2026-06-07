// Driver CLI del motor de intercambio cinético de riqueza.
//
//   socium_kwem [--n N] [--sweeps S] [--rule dy|saving|yardsale]
//               [--lambda L] [--f F] [--tax T] [--tax-every K]
//               [--seed SEED] [--threads T] [--hist archivo.csv] [--bins B]
//
// Imprime parámetros y métricas (Gini, top-1%, top-10%). Con --hist escribe un
// histograma de riqueza (binning logarítmico) para graficar en Python.
#include "socium/economy.hpp"

#include <chrono>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#ifdef _OPENMP
#include <omp.h>
#endif

using namespace socium;

static const char* arg(int argc, char** argv, const char* key, const char* def) {
    for (int i = 1; i + 1 < argc; ++i)
        if (std::strcmp(argv[i], key) == 0) return argv[i + 1];
    return def;
}

static void write_histogram(const std::vector<Real>& w, const std::string& path, int bins) {
    Real wmax = 0.0;
    for (Real x : w) wmax = std::max(wmax, x);
    if (wmax <= 0.0) return;
    const Real lo = std::log10(std::max(wmax * 1e-6, 1e-12));
    const Real hi = std::log10(wmax) + 1e-9;
    std::vector<long long> count(bins, 0);
    for (Real x : w) {
        if (x <= 0.0) continue;
        int b = static_cast<int>((std::log10(x) - lo) / (hi - lo) * bins);
        if (b < 0) b = 0;
        if (b >= bins) b = bins - 1;
        ++count[b];
    }
    std::ofstream out(path);
    out << "wealth_bin_center,count\n";
    for (int b = 0; b < bins; ++b) {
        const Real center = std::pow(10.0, lo + (b + 0.5) * (hi - lo) / bins);
        out << center << "," << count[b] << "\n";
    }
    std::cerr << "histograma -> " << path << "\n";
}

int main(int argc, char** argv) {
    EconomyParams p;
    p.n_agents  = std::atoll(arg(argc, argv, "--n",         "1000000"));
    p.n_sweeps  = std::atoll(arg(argc, argv, "--sweeps",    "1000"));
    p.rule      = rule_from_string(arg(argc, argv, "--rule", "saving"));
    p.lambda    = std::atof(arg(argc, argv, "--lambda",     "0.5"));
    p.f         = std::atof(arg(argc, argv, "--f",          "0.5"));
    p.tax_rate  = std::atof(arg(argc, argv, "--tax",        "0.0"));
    p.tax_every = std::atoll(arg(argc, argv, "--tax-every", "0"));
    p.seed      = std::strtoull(arg(argc, argv, "--seed",   "42"), nullptr, 10);

#ifdef _OPENMP
    const char* th = arg(argc, argv, "--threads", nullptr);
    if (th) omp_set_num_threads(std::atoi(th));
#endif

    std::cout << "== Socium KWEM ==\n"
              << "  agentes   : " << p.n_agents << "\n"
              << "  sweeps    : " << p.n_sweeps << "\n"
              << "  regla     : " << rule_name(p.rule) << "\n"
              << "  lambda    : " << p.lambda << "   f: " << p.f << "\n"
              << "  impuesto  : " << p.tax_rate << " cada " << p.tax_every << " sweeps\n"
#ifdef _OPENMP
              << "  hilos     : " << omp_get_max_threads() << "\n"
#else
              << "  hilos     : 1 (sin OpenMP)\n"
#endif
              ;

    const auto t0 = std::chrono::steady_clock::now();
    Economy eco(p);
    eco.run();
    const auto t1 = std::chrono::steady_clock::now();
    const double secs = std::chrono::duration<double>(t1 - t0).count();

    std::cout << "-- resultados --\n"
              << "  tiempo    : " << secs << " s\n"
              << "  <w>       : " << eco.mean_wealth() << "\n"
              << "  Gini      : " << eco.gini() << "\n"
              << "  top 1%    : " << eco.top_share(0.01) << "\n"
              << "  top 10%   : " << eco.top_share(0.10) << "\n";

    const char* hist = arg(argc, argv, "--hist", nullptr);
    if (hist) write_histogram(eco.wealth(), hist, std::atoi(arg(argc, argv, "--bins", "60")));
    return 0;
}
