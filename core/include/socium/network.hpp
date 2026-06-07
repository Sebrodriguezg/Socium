#pragma once
// Red social explícita (grafo de contactos) sobre la población. Capas: hogar (todos los
// miembros conectados) + comunidad (k vecinos aleatorios del mismo municipio, homofilia
// geográfica). Almacenamiento disperso CSR para escalar a decenas de millones de agentes.
// La usan M5 (crimen, efecto de pares) y M6 (opinión, conformidad con vecinos).
#include <cstdint>
#include <vector>

namespace socium {

struct Population;

struct Network {
    std::vector<std::int64_t> off;   // CSR: tamaño N+1
    std::vector<std::int32_t> nbr;   // CSR: vecinos (tamaño = nº de aristas dirigidas)

    std::int64_t n() const { return off.empty() ? 0 : static_cast<std::int64_t>(off.size()) - 1; }
    std::int64_t aristas() const { return static_cast<std::int64_t>(nbr.size()); }
    double grado_medio() const { return n() ? static_cast<double>(aristas()) / n() : 0.0; }
    std::size_t bytes() const { return off.size()*8 + nbr.size()*4; }
};

// Construye la red: enlaces de hogar (por hogar_id) + grado_geo vecinos aleatorios del
// mismo municipio por agente.
Network construir_red(const Population& p, int grado_geo, std::uint64_t seed = 13);

} // namespace socium
