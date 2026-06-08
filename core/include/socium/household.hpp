#pragma once
// Hogares: agrupan individuos de un mismo municipio. Estructura SoA paralela a la
// de Population. Cada persona referencia su hogar con Population.hogar_id (índice
// en estos vectores). Variables de nivel `Hg` del catálogo de métricas.
#include <cstdint>
#include <vector>

#include "socium/population.hpp"   // enums GrupoSisben, TenenciaVivienda + Population

namespace socium {

struct Geography; // fwd

enum class TipoHogar : std::uint8_t {
    Unipersonal,  // 1 persona
    Nuclear,      // con menores (pareja/jefe + hijos)
    Extenso,      // 6+ personas
    Compuesto     // adultos sin menores (2-5)
};
const char* tipo_hogar_name(TipoHogar t);

struct Households {
    std::vector<std::uint16_t>     municipio_id;
    std::vector<std::uint8_t>      departamento_id;
    std::vector<std::uint8_t>      tamano;        // nº de personas
    std::vector<std::uint8_t>      num_menores;   // miembros < 18
    std::vector<TipoHogar>         tipo;
    std::vector<float>             ingreso;       // COP/mes (suma ingresos laborales)
    std::vector<std::uint8_t>      estrato;       // 1..6
    std::vector<GrupoSisben>       sisben;        // A..D
    std::vector<TenenciaVivienda>  tenencia;
    // Privaciones materiales (IPM, consideración #3). 1 = privado. Asignadas por estrato×zona.
    std::vector<std::uint8_t>      sin_agua;          // sin fuente de agua mejorada
    std::vector<std::uint8_t>      sin_excretas;      // inadecuada eliminación de excretas
    std::vector<std::uint8_t>      piso_inadec;       // pisos inadecuados
    std::vector<std::uint8_t>      pared_inadec;      // paredes exteriores inadecuadas
    std::vector<std::uint8_t>      hacinamiento;      // hacinamiento crítico
    std::vector<std::uint8_t>      sin_internet;      // sin acceso a internet
    std::vector<std::uint8_t>      barrera_priminf;   // barreras a servicios primera infancia
    std::vector<std::uint8_t>      trabajo_infantil;  // trabajo infantil en el hogar
    std::vector<std::uint8_t>      barrera_salud;     // barreras de acceso a salud

    std::int64_t size() const { return static_cast<std::int64_t>(tamano.size()); }
};

// Forma hogares a partir de una población ya localizada (municipio asignado).
// Agrupa individuos por municipio, los reparte en hogares con tamaños realistas,
// fija Population.hogar_id y calcula los atributos de cada hogar.
Households form_households(Population& p, const Geography& g, std::uint64_t seed = 11);

namespace metrics {
Real  mean_household_size(const Households& h);
Real  gini_household_income(const Households& h);
Real  share_tipo(const Households& h, TipoHogar t);
Real  mean_estrato(const Households& h);
} // namespace metrics

} // namespace socium
