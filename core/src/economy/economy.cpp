#include "socium/economy.hpp"
#include "socium/rng.hpp"

#include <algorithm>
#include <cmath>
#include <numeric>
#include <stdexcept>
#ifdef _OPENMP
#include <omp.h>
#endif

namespace socium {

Rule rule_from_string(const std::string& s) {
    if (s == "dy" || s == "DY")                 return Rule::DragulescuYakovenko;
    if (s == "saving")                          return Rule::SavingPropensity;
    if (s == "yardsale" || s == "ys")           return Rule::YardSale;
    throw std::invalid_argument("regla desconocida: " + s);
}

const char* rule_name(Rule r) {
    switch (r) {
        case Rule::DragulescuYakovenko: return "DragulescuYakovenko";
        case Rule::SavingPropensity:    return "SavingPropensity";
        case Rule::YardSale:            return "YardSale";
    }
    return "?";
}

Economy::Economy(EconomyParams p) : p_(p) {
    if (p_.n_agents < 2) throw std::invalid_argument("n_agents debe ser >= 2");
    wealth_.assign(static_cast<std::size_t>(p_.n_agents), p_.initial_wealth);
    order_.resize(static_cast<std::size_t>(p_.n_agents));
    std::iota(order_.begin(), order_.end(), Index{0});

    // Sembrar el RNG de cada hilo de forma determinista.
    #pragma omp parallel
    { seed_thread_rng(p_.seed); }
}

// Aplica la regla de intercambio a un par (i, j) usando el RNG del hilo.
static inline void interact(Real& wi, Real& wj, const EconomyParams& p) {
    switch (p.rule) {
        case Rule::DragulescuYakovenko: {
            const Real total = wi + wj;
            const Real eps   = uniform01();
            wi = eps * total;
            wj = total - wi;
            break;
        }
        case Rule::SavingPropensity: {
            const Real total = wi + wj;
            const Real eps   = uniform01();
            const Real lam   = p.lambda;
            wi = lam * wi + eps * (1.0 - lam) * total;
            wj = total - wi;                       // conserva el total exactamente
            break;
        }
        case Rule::YardSale: {
            const Real stake = p.f * std::min(wi, wj);
            if (uniform01() < 0.5) { wi += stake; wj -= stake; }
            else                   { wi -= stake; wj += stake; }
            break;
        }
    }
}

void Economy::sweep() {
    // Emparejamiento aleatorio SIN colisiones: barajamos el orden (Fisher-Yates,
    // serial y barato) y emparejamos (2k, 2k+1). Así cada agente participa en a lo
    // sumo un par por sweep -> las transacciones son independientes y paralelizables
    // sin condiciones de carrera.
    auto& rng = thread_rng();
    for (Index i = p_.n_agents - 1; i > 0; --i) {
        std::uniform_int_distribution<Index> d(0, i);
        std::swap(order_[i], order_[d(rng)]);
    }

    const Index n_pairs = p_.n_agents / 2;
    #pragma omp parallel for schedule(static)
    for (Index k = 0; k < n_pairs; ++k) {
        const Index i = order_[2 * k];
        const Index j = order_[2 * k + 1];
        interact(wealth_[i], wealth_[j], p_);
    }
}

void Economy::apply_tax_redistribution() {
    const Real tau = p_.tax_rate;
    if (tau <= 0.0) return;

    // Recauda tau*w de cada agente y redistribuye en partes iguales.
    Real pool = 0.0;
    #pragma omp parallel for reduction(+:pool) schedule(static)
    for (Index i = 0; i < p_.n_agents; ++i) {
        const Real t = tau * wealth_[i];
        wealth_[i]  -= t;
        pool        += t;
    }
    const Real per_capita = pool / static_cast<Real>(p_.n_agents);
    #pragma omp parallel for schedule(static)
    for (Index i = 0; i < p_.n_agents; ++i)
        wealth_[i] += per_capita;
}

void Economy::run() {
    for (Index s = 0; s < p_.n_sweeps; ++s) {
        sweep();
        if (p_.tax_every > 0 && (s + 1) % p_.tax_every == 0)
            apply_tax_redistribution();
    }
}

Real Economy::mean_wealth() const {
    Real sum = 0.0;
    #pragma omp parallel for reduction(+:sum) schedule(static)
    for (Index i = 0; i < p_.n_agents; ++i) sum += wealth_[i];
    return sum / static_cast<Real>(p_.n_agents);
}

Real Economy::gini() const {
    // G = (2 * sum_{i} i*w_(i)) / (n * sum w) - (n+1)/n,  con w ordenado ascendente.
    std::vector<Real> w = wealth_;
    std::sort(w.begin(), w.end());
    const Index n = p_.n_agents;
    long double weighted = 0.0L, total = 0.0L;
    for (Index i = 0; i < n; ++i) {
        weighted += static_cast<long double>(i + 1) * w[i];
        total    += w[i];
    }
    if (total <= 0.0L) return 0.0;
    return static_cast<Real>((2.0L * weighted) / (n * total) - static_cast<long double>(n + 1) / n);
}

Real Economy::top_share(Real fraction) const {
    std::vector<Real> w = wealth_;
    std::sort(w.begin(), w.end(), std::greater<Real>());
    const Index cut = std::max<Index>(1, static_cast<Index>(fraction * p_.n_agents));
    long double top = 0.0L, total = 0.0L;
    for (Index i = 0; i < p_.n_agents; ++i) {
        total += w[i];
        if (i < cut) top += w[i];
    }
    return total > 0.0L ? static_cast<Real>(top / total) : 0.0;
}

} // namespace socium
