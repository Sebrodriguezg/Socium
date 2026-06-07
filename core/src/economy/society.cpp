#include "socium/society.hpp"
#include "socium/rng.hpp"

#include <algorithm>
#include <numeric>
#include <random>
#include <vector>

namespace socium {

// Un barrido mean-field: baraja todo y empareja consecutivos.
static void sweep_global(Population& p, std::vector<std::int64_t>& order,
                         Rule rule, Real lambda, Real f) {
    auto& rng = thread_rng();
    for (std::int64_t i = p.size() - 1; i > 0; --i) {
        std::uniform_int_distribution<std::int64_t> d(0, i);
        std::swap(order[i], order[d(rng)]);
    }
    const std::int64_t np = p.size() / 2;
    #pragma omp parallel for schedule(static)
    for (std::int64_t k = 0; k < np; ++k) {
        const std::int64_t i = order[2 * k], j = order[2 * k + 1];
        Real a = p.riqueza[i], b = p.riqueza[j];
        apply_exchange(a, b, rule, lambda, f);
        p.riqueza[i] = static_cast<float>(a);
        p.riqueza[j] = static_cast<float>(b);
    }
}

void simular_economia(Population& p, const EconomyParams& ep, std::int64_t n_sweeps,
                      bool por_municipio, std::uint64_t seed) {
    const std::int64_t N = p.size();
    if (N < 2) return;

    #pragma omp parallel
    { seed_thread_rng(seed); }

    std::vector<std::int64_t> order(static_cast<std::size_t>(N));
    std::iota(order.begin(), order.end(), std::int64_t{0});

    if (!por_municipio) {
        for (std::int64_t s = 0; s < n_sweeps; ++s)
            sweep_global(p, order, ep.rule, ep.lambda, ep.f);
        return;
    }

    // Agrupar individuos por municipio una sola vez (counting sort).
    std::int64_t M = 0;
    for (std::int64_t i = 0; i < N; ++i) M = std::max<std::int64_t>(M, p.municipio_id[i] + 1);
    std::vector<std::int64_t> off(static_cast<std::size_t>(M) + 1, 0);
    for (std::int64_t i = 0; i < N; ++i) off[p.municipio_id[i] + 1]++;
    for (std::int64_t m = 0; m < M; ++m) off[m + 1] += off[m];
    std::vector<std::int64_t> cur(off.begin(), off.end() - 1);
    for (std::int64_t i = 0; i < N; ++i) order[cur[p.municipio_id[i]]++] = i;

    for (std::int64_t s = 0; s < n_sweeps; ++s) {
        // cada municipio es independiente -> paralelizable sin condiciones de carrera
        #pragma omp parallel for schedule(dynamic, 8)
        for (std::int64_t m = 0; m < M; ++m) {
            const std::int64_t b = off[m], e = off[m + 1], L = e - b;
            if (L < 2) continue;
            auto& rng = thread_rng();
            for (std::int64_t k = L - 1; k > 0; --k) {       // Fisher-Yates en el bloque
                std::uniform_int_distribution<std::int64_t> d(0, k);
                std::swap(order[b + k], order[b + d(rng)]);
            }
            for (std::int64_t k = 0; k + 1 < L; k += 2) {
                const std::int64_t i = order[b + k], j = order[b + k + 1];
                Real a = p.riqueza[i], bb = p.riqueza[j];
                apply_exchange(a, bb, ep.rule, ep.lambda, ep.f);
                p.riqueza[i] = static_cast<float>(a);
                p.riqueza[j] = static_cast<float>(bb);
            }
        }
    }
}

namespace metrics {

Real gini_riqueza(const Population& p) {
    std::vector<float> w(p.riqueza);
    std::sort(w.begin(), w.end());
    const std::int64_t n = static_cast<std::int64_t>(w.size());
    long double weighted = 0.0L, total = 0.0L;
    for (std::int64_t i = 0; i < n; ++i) { weighted += static_cast<long double>(i + 1) * w[i]; total += w[i]; }
    if (total <= 0.0L) return 0.0;
    return static_cast<Real>((2.0L * weighted) / (n * total) - static_cast<long double>(n + 1) / n);
}

Real top_share_riqueza(const Population& p, Real frac) {
    std::vector<float> w(p.riqueza);
    std::sort(w.begin(), w.end(), std::greater<float>());
    const std::int64_t n = static_cast<std::int64_t>(w.size());
    const std::int64_t cut = std::max<std::int64_t>(1, static_cast<std::int64_t>(frac * n));
    long double top = 0.0L, total = 0.0L;
    for (std::int64_t i = 0; i < n; ++i) { total += w[i]; if (i < cut) top += w[i]; }
    return total > 0.0L ? static_cast<Real>(top / total) : 0.0;
}

} // namespace metrics
} // namespace socium
