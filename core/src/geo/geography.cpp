#include "socium/geography.hpp"

#include <fstream>
#include <sstream>
#include <stdexcept>

namespace socium {

// Divide una línea CSV respetando comillas dobles (los nombres pueden traer comas,
// p.ej. "BOGOTÁ, D.C.").
static std::vector<std::string> split_csv(const std::string& line) {
    std::vector<std::string> out;
    std::string cur;
    bool quoted = false;
    for (std::size_t i = 0; i < line.size(); ++i) {
        const char c = line[i];
        if (quoted) {
            if (c == '"') {
                if (i + 1 < line.size() && line[i + 1] == '"') { cur += '"'; ++i; }
                else quoted = false;
            } else cur += c;
        } else {
            if (c == '"') quoted = true;
            else if (c == ',') { out.push_back(cur); cur.clear(); }
            else cur += c;
        }
    }
    out.push_back(cur);
    return out;
}

static float to_float(const std::string& s) {
    if (s.empty()) return 0.0f;
    try { return std::stof(s); } catch (...) { return 0.0f; }
}

Geography Geography::load_csv(const std::string& path) {
    std::ifstream in(path);
    if (!in) throw std::runtime_error("no se pudo abrir DIVIPOLA: " + path);

    Geography g;
    std::string line;
    std::getline(in, line); // encabezado

    while (std::getline(in, line)) {
        if (line.empty()) continue;
        auto f = split_csv(line);
        if (f.size() < 7) continue;
        const std::string& cod_dpto = f[0];
        const std::string& dpto     = f[1];
        const std::string& cod_mpio = f[2];
        const std::string& nom_mpio = f[3];
        const std::string& tipo     = f[4];

        // registra departamento si es nuevo
        auto it = g.idx_dpto.find(cod_dpto);
        std::uint8_t di;
        if (it == g.idx_dpto.end()) {
            di = static_cast<std::uint8_t>(g.dpto_codigo.size());
            g.idx_dpto.emplace(cod_dpto, di);
            g.dpto_codigo.push_back(cod_dpto);
            g.dpto_nombre.push_back(dpto);
        } else {
            di = it->second;
        }

        const std::uint16_t mi = static_cast<std::uint16_t>(g.mpio_codigo.size());
        g.idx_mpio.emplace(cod_mpio, mi);
        g.mpio_codigo.push_back(cod_mpio);
        g.mpio_nombre.push_back(nom_mpio);
        g.mpio_tipo.push_back(tipo);
        g.mpio_dpto.push_back(di);
        g.mpio_lon.push_back(to_float(f[5]));
        g.mpio_lat.push_back(to_float(f[6]));
    }
    return g;
}

std::int64_t Geography::cargar_pesos(const std::string& path) {
    std::ifstream in(path);
    if (!in) throw std::runtime_error("no se pudo abrir municipios.csv: " + path);
    mpio_peso.assign(mpio_codigo.size(), 0.0);
    mpio_urbano.assign(mpio_codigo.size(), 0.7f);   // default si el archivo no trae pct

    std::string line;
    std::getline(in, line); // encabezado: cod_mpio,poblacion_total,...,pct_urbano
    std::int64_t con_peso = 0;
    while (std::getline(in, line)) {
        if (line.empty()) continue;
        auto f = split_csv(line);
        if (f.size() < 2) continue;
        auto it = idx_mpio.find(f[0]);
        if (it == idx_mpio.end()) continue;
        const double w = to_float(f[1]);
        if (w > 0.0) { mpio_peso[it->second] = w; ++con_peso; }
        if (f.size() >= 5) mpio_urbano[it->second] = to_float(f[4]) / 100.0f;  // pct_urbano
    }
    return con_peso;
}

std::int64_t Geography::cargar_conflicto(const std::string& path) {
    mpio_conflicto.assign(mpio_codigo.size(), 0.0f);
    std::ifstream in(path);
    if (!in) return 0;  // opcional: si no existe, conflicto = 0
    std::string line;
    std::getline(in, line); // encabezado: cod_mpio,homicidios_anual,tasa_homicidios_100k,indice_conflicto
    std::int64_t con = 0;
    while (std::getline(in, line)) {
        if (line.empty()) continue;
        auto f = split_csv(line);
        if (f.size() < 4) continue;
        auto it = idx_mpio.find(f[0]);
        if (it == idx_mpio.end()) continue;
        mpio_conflicto[it->second] = to_float(f[3]);
        ++con;
    }
    return con;
}

std::int64_t Geography::cargar_pib(const std::string& path) {
    dpto_productividad.assign(dpto_codigo.size(), 1.0f);
    std::ifstream in(path);
    if (!in) return 0;
    std::string line;
    std::getline(in, line); // encabezado: cod_dpto,pib_pc_cop,ratio
    std::int64_t con = 0;
    while (std::getline(in, line)) {
        if (line.empty()) continue;
        auto f = split_csv(line);
        if (f.size() < 3) continue;
        auto it = idx_dpto.find(f[0]);
        if (it == idx_dpto.end()) continue;
        dpto_productividad[it->second] = to_float(f[2]);
        ++con;
    }
    return con;
}

std::vector<std::int64_t> Geography::conteo_municipios_por_depto() const {
    std::vector<std::int64_t> c(dpto_codigo.size(), 0);
    for (std::uint8_t d : mpio_dpto) ++c[d];
    return c;
}

} // namespace socium
