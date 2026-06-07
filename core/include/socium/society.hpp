#pragma once
// Acopla el motor económico (KWEM) a la población real: las transacciones operan
// sobre Population.riqueza. Modo `por_municipio` => los agentes solo transaccionan
// con otros de su mismo municipio (interacción local, no mean-field).
#include <cstdint>

#include "socium/economy.hpp"     // EconomyParams, Rule, apply_exchange
#include "socium/population.hpp"  // Population

namespace socium {

// Corre n_sweeps de intercambio de riqueza sobre p.riqueza.
// por_municipio = true  -> emparejamiento solo dentro del mismo municipio.
// por_municipio = false -> mean-field (cualquier par del país).
void simular_economia(Population& p, const EconomyParams& ep, std::int64_t n_sweeps,
                      bool por_municipio = true, std::uint64_t seed = 99);

namespace metrics {
Real gini_riqueza(const Population& p);
Real top_share_riqueza(const Population& p, Real frac);
} // namespace metrics

} // namespace socium
