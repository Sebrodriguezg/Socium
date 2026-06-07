#pragma once
// Motor de intercambio cinético de riqueza (Kinetic Wealth-Exchange Model).
// Refs: Drăgulescu-Yakovenko 2000 (DY), Chakraborti-Chakrabarti 2000 (ahorro),
//       Boghosian 2014 (Yard-Sale), Bisi-Spiga-Toscani 2009 (impuesto+redistribución).
// Ver docs/theory/bibliography-review.md
#include <algorithm>
#include <cstdint>
#include <string>
#include <vector>

#include "socium/rng.hpp"

namespace socium {

using Real  = double;
using Index = std::int64_t;

enum class Rule {
    DragulescuYakovenko, // reparto aleatorio del total del par
    SavingPropensity,    // cada agente guarda una fracción lambda
    YardSale             // apuesta una fracción del menor patrimonio
};

Rule        rule_from_string(const std::string& s);
const char* rule_name(Rule r);

// Aplica una transacción de intercambio de riqueza a un par (wi, wj). Conserva el
// total del par. Inline para reutilizar tanto en el motor mean-field (Economy) como
// en la economía acoplada a la población real. Usa el RNG del hilo.
inline void apply_exchange(Real& wi, Real& wj, Rule rule, Real lambda, Real f) {
    switch (rule) {
        case Rule::DragulescuYakovenko: {
            const Real total = wi + wj;
            wi = uniform01() * total;
            wj = total - wi;
            break;
        }
        case Rule::SavingPropensity: {
            const Real total = wi + wj;
            wi = lambda * wi + uniform01() * (1.0 - lambda) * total;
            wj = total - wi;
            break;
        }
        case Rule::YardSale: {
            const Real stake = f * std::min(wi, wj);
            if (uniform01() < 0.5) { wi += stake; wj -= stake; }
            else                   { wi -= stake; wj += stake; }
            break;
        }
    }
}

struct EconomyParams {
    Index n_agents       = 1'000'000;
    Index n_sweeps       = 1'000;     // un "sweep" = N/2 transacciones (cada agente ~1 vez)
    Real  initial_wealth = 1.0;
    Rule  rule           = Rule::SavingPropensity;
    Real  lambda         = 0.5;       // propensión al ahorro (regla SavingPropensity)
    Real  f              = 0.5;       // fracción apostada (regla YardSale)
    // --- política fiscal ---
    Real  tax_rate       = 0.0;       // impuesto plano sobre patrimonio por aplicación
    Index tax_every      = 0;         // aplicar cada k sweeps (0 = nunca)
    std::uint64_t seed   = 42;
};

class Economy {
public:
    explicit Economy(EconomyParams p);

    void run();                                   // ejecuta todos los sweeps
    void sweep();                                 // un barrido de transacciones

    // métricas
    Real mean_wealth() const;
    Real gini() const;                            // [0,1]; 0 = igualdad total
    Real top_share(Real fraction) const;          // fracción de riqueza del top `fraction`

    const std::vector<Real>& wealth() const { return wealth_; }
    const EconomyParams&     params() const { return p_; }

private:
    void apply_tax_redistribution();

    EconomyParams      p_;
    std::vector<Real>  wealth_;
    std::vector<Index> order_;   // permutación para emparejar sin colisiones
};

} // namespace socium
