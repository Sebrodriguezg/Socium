#include "socium/firm.hpp"
#include "socium/geography.hpp"
#include "socium/rng.hpp"

#include <algorithm>
#include <cmath>
#include <random>

namespace socium {

// Distribución de tamaño (DANE EMTSE): micro 92%, pequeña 5%, mediana 2%, grande 1%.
static const double TAM_CDF[4] = {0.92, 0.97, 0.99, 1.00};
// Distribución sectorial aproximada (DANE).
static const double SEC_CDF[7] = {0.08, 0.52, 0.63, 0.88, 0.96, 0.98, 1.00};
//                                agro  comerc indus servic constr miner  otro

static int empleos_por_tamano(TamanoEmpresa t, std::mt19937_64& rng) {
    std::uniform_real_distribution<double> U(0.0, 1.0);
    switch (t) {
        case TamanoEmpresa::Micro:   return 1 + static_cast<int>(U(rng) * U(rng) * 9);   // 1-10, sesgo bajo
        case TamanoEmpresa::Pequena: return 11 + static_cast<int>(U(rng) * 39);          // 11-50
        case TamanoEmpresa::Mediana: return 51 + static_cast<int>(U(rng) * 149);         // 51-200
        case TamanoEmpresa::Grande:  return 201 + static_cast<int>(U(rng) * 799);        // 201-1000
    }
    return 1;
}

Firms crear_empresas(const Geography& g, std::int64_t target_empleos, std::uint64_t seed) {
    Firms f;
    const std::int64_t M = g.n_municipios();
    if (M == 0 || g.mpio_peso.empty()) return f;

    // CDF de población para ubicar empresas donde está la gente
    std::vector<double> cum(static_cast<std::size_t>(M));
    double acc = 0.0;
    for (std::int64_t m = 0; m < M; ++m) { acc += g.mpio_peso[m]; cum[m] = acc; }
    if (acc <= 0.0) return f;

    // empleos promedio por empresa ≈ 10.7 con la distribución anterior
    const double avg_empleos = 0.92 * 3 + 0.05 * 25 + 0.02 * 110 + 0.01 * 450;
    std::int64_t n = std::max<std::int64_t>(1, static_cast<std::int64_t>(target_empleos / avg_empleos));

    std::mt19937_64 rng(splitmix64(seed ^ 0xF1F1F1ULL));
    std::uniform_real_distribution<double> U(0.0, 1.0);
    f.municipio.reserve(n); f.departamento.reserve(n); f.tamano.reserve(n);
    f.sector.reserve(n); f.capital.reserve(n); f.empleos.reserve(n);
    f.formal.reserve(n); f.activa.reserve(n);

    std::int64_t empleos_acum = 0;
    while (empleos_acum < target_empleos) {
        double u = U(rng);
        TamanoEmpresa tam = u < TAM_CDF[0] ? TamanoEmpresa::Micro
                          : u < TAM_CDF[1] ? TamanoEmpresa::Pequena
                          : u < TAM_CDF[2] ? TamanoEmpresa::Mediana : TamanoEmpresa::Grande;
        double us = U(rng);
        int si = 0; while (si < 6 && us > SEC_CDF[si]) ++si;

        std::int64_t m = std::lower_bound(cum.begin(), cum.end(), U(rng) * acc) - cum.begin();
        if (m >= M) m = M - 1;
        const int emp = empleos_por_tamano(tam, rng);

        // capital lognormal según tamaño (COP)
        const double mu = (tam == TamanoEmpresa::Micro) ? 16.3 :
                          (tam == TamanoEmpresa::Pequena) ? 18.5 :
                          (tam == TamanoEmpresa::Mediana) ? 20.5 : 22.8;
        std::lognormal_distribution<double> LN(mu, 0.8);
        // formalidad por tamaño (micro 33%, peq 70%, med 95%, grande 100%)
        const double pf = (tam == TamanoEmpresa::Micro) ? 0.33 :
                          (tam == TamanoEmpresa::Pequena) ? 0.70 :
                          (tam == TamanoEmpresa::Mediana) ? 0.95 : 1.00;

        f.municipio.push_back(static_cast<std::uint16_t>(m));
        f.departamento.push_back(g.mpio_dpto[static_cast<std::size_t>(m)]);
        f.tamano.push_back(tam);
        f.sector.push_back(static_cast<SectorEmpresa>(si));
        f.capital.push_back(static_cast<float>(LN(rng)));
        f.empleos.push_back(emp);
        f.formal.push_back(U(rng) < pf ? 1 : 0);
        f.activa.push_back(1);
        empleos_acum += emp;
    }
    return f;
}

namespace metrics {

std::int64_t empresas_activas(const Firms& f) {
    std::int64_t c = 0;
    for (std::int64_t i = 0; i < f.size(); ++i) c += f.activa[i];
    return c;
}

std::int64_t empleos_ofrecidos(const Firms& f) {
    std::int64_t e = 0;
    for (std::int64_t i = 0; i < f.size(); ++i) if (f.activa[i]) e += f.empleos[i];
    return e;
}

Real informalidad_empresarial(const Firms& f) {
    std::int64_t act = 0, inf = 0;
    for (std::int64_t i = 0; i < f.size(); ++i)
        if (f.activa[i]) { ++act; inf += (f.formal[i] == 0); }
    return act ? static_cast<Real>(inf) / act : 0.0;
}

} // namespace metrics
} // namespace socium
