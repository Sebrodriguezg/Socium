#pragma once
// Geografía de Colombia (DIVIPOLA, DANE): departamentos y municipios reales.
// Carga data/reference/divipola.csv. Los índices internos (0..N-1) son los que
// usan los agentes en Population.municipio_id / departamento_id.
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace socium {

struct Geography {
    // departamentos (índice 0..n_dptos-1)
    std::vector<std::string> dpto_codigo;   // código DANE ("05")
    std::vector<std::string> dpto_nombre;   // "ANTIOQUIA"

    // municipios / entidades (índice 0..n_mpios-1)
    std::vector<std::string>   mpio_codigo; // código DANE ("05001")
    std::vector<std::string>   mpio_nombre; // "MEDELLÍN"
    std::vector<std::string>   mpio_tipo;   // Municipio / Isla / Área no municipalizada
    std::vector<std::uint8_t>  mpio_dpto;   // índice del departamento
    std::vector<float>         mpio_lon;
    std::vector<float>         mpio_lat;
    std::vector<double>        mpio_peso;     // tamaño poblacional (población total)
    std::vector<float>         mpio_urbano;   // fracción urbana [0,1] (pct cabecera/100)

    // búsqueda por código DANE
    std::unordered_map<std::string, std::uint8_t>  idx_dpto;
    std::unordered_map<std::string, std::uint16_t> idx_mpio;

    std::int64_t n_departamentos() const { return static_cast<std::int64_t>(dpto_codigo.size()); }
    std::int64_t n_municipios()    const { return static_cast<std::int64_t>(mpio_codigo.size()); }

    // nº de municipios por departamento (tamaño = n_departamentos)
    std::vector<std::int64_t> conteo_municipios_por_depto() const;

    static Geography load_csv(const std::string& path);

    // Carga pesos poblacionales por municipio desde data/reference/municipios.csv
    // (columna poblacion_5_16). Devuelve cuántos municipios quedaron con peso > 0.
    std::int64_t cargar_pesos(const std::string& path);
};

} // namespace socium
