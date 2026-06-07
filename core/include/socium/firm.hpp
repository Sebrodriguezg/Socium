#pragma once
// Agente EMPRESA / unidad productiva (spec §3). Las empresas crean los puestos de
// trabajo (empleo endógeno), pagan extorsión en zonas de conflicto y pueden quebrar.
// Cadena: conflicto -> extorsión -> quiebra -> menos empleos -> desempleo.
#include <cstdint>
#include <vector>

#include "socium/economy.hpp"  // Real

namespace socium {

struct Geography;

enum class TamanoEmpresa : std::uint8_t { Micro, Pequena, Mediana, Grande };
enum class SectorEmpresa : std::uint8_t {
    Agro, Comercio, Industria, Servicios, Construccion, Mineria, Otro
};

struct Firms {
    std::vector<std::uint16_t>   municipio;
    std::vector<std::uint8_t>    departamento;
    std::vector<TamanoEmpresa>   tamano;
    std::vector<SectorEmpresa>   sector;
    std::vector<float>           capital;    // COP
    std::vector<std::int32_t>    empleos;    // puestos de trabajo que ofrece
    std::vector<std::uint8_t>    formal;     // 0/1 (registro)
    std::vector<std::uint8_t>    activa;     // 1 operando, 0 quebrada

    std::int64_t size() const { return static_cast<std::int64_t>(municipio.size()); }
};

// Crea empresas con la distribución de tamaño DANE (§3: micro 92%, peq 5%, med 2%,
// grande 1%), ubicadas por peso poblacional. El total de empleos ≈ target_empleos.
Firms crear_empresas(const Geography& g, std::int64_t target_empleos, std::uint64_t seed);

namespace metrics {
std::int64_t empresas_activas(const Firms& f);
std::int64_t empleos_ofrecidos(const Firms& f);   // de empresas activas
Real informalidad_empresarial(const Firms& f);
} // namespace metrics

} // namespace socium
