// Demo de la geografía DIVIPOLA.
//   socium_geo [ruta_csv]   (por defecto data/reference/divipola.csv)
#include "socium/geography.hpp"

#include <iostream>

using namespace socium;

int main(int argc, char** argv) {
    const std::string path = (argc > 1) ? argv[1] : "data/reference/divipola.csv";
    Geography g = Geography::load_csv(path);

    std::cout << "== Socium · Geografía (DIVIPOLA) ==\n"
              << "  departamentos : " << g.n_departamentos() << "\n"
              << "  municipios    : " << g.n_municipios() << "\n\n"
              << "  municipios por departamento:\n";

    auto conteo = g.conteo_municipios_por_depto();
    for (std::int64_t d = 0; d < g.n_departamentos(); ++d)
        std::cout << "    " << g.dpto_codigo[d] << " " << g.dpto_nombre[d]
                  << " : " << conteo[d] << "\n";

    // ejemplo de búsqueda por código DANE
    auto it = g.idx_mpio.find("11001");
    if (it != g.idx_mpio.end()) {
        const auto m = it->second;
        std::cout << "\n  ejemplo 11001 -> " << g.mpio_nombre[m]
                  << " (" << g.dpto_nombre[g.mpio_dpto[m]] << "), "
                  << "lat " << g.mpio_lat[m] << " lon " << g.mpio_lon[m] << "\n";
    }
    return 0;
}
