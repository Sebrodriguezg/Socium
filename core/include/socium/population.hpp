#pragma once
// Substrato de agentes: la POBLACIÓN de individuos (personas) de Colombia.
// Diseño Struct-of-Arrays (SoA), cache-friendly y compacto para escalar a ~52M
// personas en memoria (ADR 0002). Cada campo mapea a una variable de estado `E`
// del catálogo (docs/model-spec/metrics-catalog.md / metrics.yaml).
//
// Tipos compactos a propósito: uint8 para categóricas/edades, float para dinero,
// int32 para id de hogar, uint16 para municipio (~1.103), uint8 para depto (33).
#include <cstdint>
#include <vector>

namespace socium {

using Real = double;

// --- Categóricas (enum class uint8_t) — orden = código interno ---
enum class Sexo : std::uint8_t { Hombre, Mujer };
enum class Etnia : std::uint8_t { Ninguna, Indigena, NARP, Rrom };
enum class EstadoCivil : std::uint8_t { Soltero, UnionLibre, Casado, Separado, Viudo };
enum class NivelEducativo : std::uint8_t {
    Ninguno, Preescolar, Primaria, Secundaria, Media, Tecnico, Universitario, Posgrado
};
enum class SituacionLaboral : std::uint8_t { MenorEdad, Inactivo, Ocupado, Desocupado };
enum class Sector : std::uint8_t {
    NoAplica, Agro, Mineria, Industria, Construccion, Comercio, Servicios, Gobierno
};
enum class PosicionOcupacional : std::uint8_t {
    NoAplica, Asalariado, CuentaPropia, Empleador, SinRemuneracion
};
enum class AfiliacionSalud : std::uint8_t { Ninguno, Contributivo, Subsidiado, Especial };
enum class GrupoSisben : std::uint8_t { NoClasificado, A, B, C, D };
enum class TenenciaVivienda : std::uint8_t { Propia, Arriendo, Otra };
enum class EstatusMigratorio : std::uint8_t { Nacional, MigranteRegular, MigranteIrregular };
enum class Religiosidad : std::uint8_t { Ninguna, Catolica, Cristiana, Otra };

// Población de individuos en SoA. Todos los vectores tienen tamaño N.
struct Population {
    // demografía
    std::vector<std::uint8_t>      edad;              // años
    std::vector<Sexo>              sexo;
    std::vector<Etnia>             etnia;
    std::vector<EstadoCivil>       estado_civil;
    // educación
    std::vector<NivelEducativo>    nivel_educativo;
    std::vector<std::uint8_t>      anios_escolaridad;
    std::vector<std::uint8_t>      asiste_escuela;    // 0/1
    // trabajo
    std::vector<SituacionLaboral>  situacion_laboral;
    std::vector<Sector>            sector;
    std::vector<PosicionOcupacional> posicion;
    std::vector<std::uint8_t>      horas_trabajadas;  // horas/semana
    std::vector<std::uint8_t>      informal;          // 0/1 (clave en CO)
    std::vector<std::uint8_t>      cotiza_pension;    // 0/1
    // económico (individual)
    std::vector<float>             ingreso_laboral;   // COP/mes
    std::vector<float>             riqueza;           // patrimonio (stock)
    std::vector<float>             deuda;             // deuda del hogar (stock, COP)
    // salud
    std::vector<AfiliacionSalud>   afiliacion_salud;
    // social / político
    std::vector<std::uint8_t>      confianza;         // 0..100
    std::vector<std::uint8_t>      participa_org;     // 0/1
    std::vector<Religiosidad>      religiosidad;
    std::vector<std::int8_t>       opinion_politica;  // -100 (izq) .. +100 (der)
    std::vector<std::uint8_t>      satisfaccion_vida; // 0..10
    // migración
    std::vector<EstatusMigratorio> estatus_migratorio;
    // estados dinámicos (motor de tiempo)
    std::vector<std::uint8_t>      vivo;            // 1 vivo, 0 fallecido
    std::vector<std::uint8_t>      es_delincuente;  // 0/1 (emerge por M5)
    std::vector<std::uint8_t>      meses_enfermo;   // >0 si cursa enfermedad (M4)
    // enlaces estructurales
    std::vector<std::int32_t>      hogar_id;
    std::vector<std::uint16_t>     municipio_id;      // 0..1102 (DIVIPOLA reindexado)
    std::vector<std::uint8_t>      departamento_id;   // 0..32

    std::int64_t size() const { return static_cast<std::int64_t>(edad.size()); }
    void resize(std::int64_t n);
    // Añade un recién nacido (edad 0) en un hogar/municipio dados. Devuelve su índice.
    std::int64_t nacer(Sexo s, std::int32_t hogar, std::uint16_t municipio, std::uint8_t depto);
    // memoria aproximada por individuo (bytes), para planear corridas grandes.
    static std::size_t bytes_per_agent();
};

// Generador sintético (PLACEHOLDER hasta calibrar con Censo 2018 + GEIH).
// Produce una población con distribuciones aproximadas a Colombia para poder
// desarrollar/medir mientras se integran los datos reales.
Population build_synthetic(std::int64_t n, std::uint64_t seed = 42);

struct Geography; // fwd

// Asigna a cada agente un municipio (y su departamento) muestreando según el peso
// poblacional de Geography::mpio_peso. Distribución espacial realista de los agentes.
void asignar_municipios(Population& p, const Geography& g, std::uint64_t seed = 7);

// Ajusta los años de escolaridad según la ruralidad del municipio (urbano ~10.8 años,
// rural ~6.4; spec §2.3) para reproducir la desigualdad educativa espacial real.
void ajustar_educacion_espacial(Population& p, const Geography& g);

// --- métricas agregadas (rol `A`) ---
namespace metrics {
Real mean_age(const Population& p);
Real share_female(const Population& p);
Real unemployment_rate(const Population& p);     // desocupados / PEA
Real informality_rate(const Population& p);      // informales / ocupados
Real mean_labor_income(const Population& p);     // sobre ocupados
Real gini_income(const Population& p);           // Gini del ingreso laboral
} // namespace metrics

} // namespace socium
