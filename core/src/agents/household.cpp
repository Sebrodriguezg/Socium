#include "socium/household.hpp"
#include "socium/geography.hpp"
#include "socium/rng.hpp"

#include <algorithm>
#include <vector>

namespace socium {

const char* tipo_hogar_name(TipoHogar t) {
    switch (t) {
        case TipoHogar::Unipersonal: return "Unipersonal";
        case TipoHogar::Nuclear:     return "Nuclear";
        case TipoHogar::Extenso:     return "Extenso";
        case TipoHogar::Compuesto:   return "Compuesto";
    }
    return "?";
}

// Distribución de tamaño de hogar (aproximada a Colombia). Índice 0 -> 1 persona.
static const double SIZE_CDF[6] = {0.18, 0.39, 0.61, 0.81, 0.92, 1.00}; // 1..6+
static int sample_size(double u) {
    for (int s = 0; s < 6; ++s) if (u <= SIZE_CDF[s]) return s + 1;
    return 6;
}

static constexpr double SMLV = 1'300'000.0; // salario mínimo aprox (referencia)

// Tasa de privación material por estrato (1..6) y zona, para el IPM (consideración #3).
// Calibrado a la brecha urbano-rural del DANE (rural >> urbano). Ver data/reference/
// privaciones_ipm.csv y docs/plan-consideracion-3.md. Índice 0..5 = estrato 1..6.
struct TasasPriv { double urb[6]; double rur[6]; };
static const TasasPriv PRIV[9] = {
  /*sin_agua*/         {{0.10,0.05,0.02,0.005,0.00,0.00},{0.45,0.30,0.12,0.04,0.01,0.00}},
  /*sin_excretas*/     {{0.12,0.06,0.02,0.005,0.00,0.00},{0.50,0.32,0.12,0.04,0.01,0.00}},
  /*piso_inadec*/      {{0.10,0.05,0.01,0.005,0.00,0.00},{0.35,0.20,0.06,0.02,0.00,0.00}},
  /*pared_inadec*/     {{0.05,0.02,0.005,0.00,0.00,0.00},{0.15,0.08,0.02,0.005,0.00,0.00}},
  /*hacinamiento*/     {{0.22,0.12,0.05,0.02,0.005,0.00},{0.28,0.18,0.08,0.03,0.01,0.00}},
  /*sin_internet*/     {{0.50,0.30,0.15,0.05,0.02,0.00},{0.82,0.65,0.40,0.20,0.08,0.02}},
  /*barrera_priminf*/  {{0.16,0.10,0.05,0.02,0.01,0.00},{0.32,0.22,0.10,0.04,0.01,0.00}},
  /*trabajo_infantil*/ {{0.04,0.02,0.01,0.005,0.00,0.00},{0.11,0.07,0.03,0.01,0.00,0.00}},
  /*barrera_salud*/    {{0.10,0.07,0.05,0.03,0.02,0.01},{0.18,0.13,0.08,0.04,0.02,0.01}},
};

Households form_households(Population& p, const Geography& g, std::uint64_t seed) {
    const std::int64_t N = p.size();
    const std::int64_t M = g.n_municipios();
    Households h;
    if (N == 0 || M == 0) return h;

    // --- counting sort de individuos por municipio (O(N+M)) ---
    std::vector<std::int64_t> off(static_cast<std::size_t>(M) + 1, 0);
    for (std::int64_t i = 0; i < N; ++i) off[p.municipio_id[i] + 1]++;
    for (std::int64_t m = 0; m < M; ++m) off[m + 1] += off[m];
    std::vector<std::int64_t> order(static_cast<std::size_t>(N));
    std::vector<std::int64_t> cur(off.begin(), off.end() - 1);
    for (std::int64_t i = 0; i < N; ++i) order[cur[p.municipio_id[i]]++] = i;

    // estimación de nº de hogares para reservar (~3 personas/hogar)
    h.municipio_id.reserve(static_cast<std::size_t>(N / 3 + M));
    h.departamento_id.reserve(static_cast<std::size_t>(N / 3 + M));
    h.tamano.reserve(static_cast<std::size_t>(N / 3 + M));
    h.num_menores.reserve(static_cast<std::size_t>(N / 3 + M));
    h.tipo.reserve(static_cast<std::size_t>(N / 3 + M));
    h.ingreso.reserve(static_cast<std::size_t>(N / 3 + M));
    h.estrato.reserve(static_cast<std::size_t>(N / 3 + M));
    h.sisben.reserve(static_cast<std::size_t>(N / 3 + M));
    h.tenencia.reserve(static_cast<std::size_t>(N / 3 + M));

    seed_thread_rng(seed); // formación serial: un solo hilo
    auto& rng = thread_rng();
    std::uniform_real_distribution<double> U(0.0, 1.0);

    for (std::int64_t m = 0; m < M; ++m) {
        std::int64_t idx = off[m];
        const std::int64_t end = off[m + 1];
        const std::uint8_t depto = end > idx ? p.departamento_id[order[idx]] : 0;

        while (idx < end) {
            int s = sample_size(U(rng));
            s = static_cast<int>(std::min<std::int64_t>(s, end - idx)); // último hogar truncado
            const std::int64_t hid = h.size();

            float ingreso = 0.0f;
            std::uint8_t menores = 0;
            for (int k = 0; k < s; ++k) {
                const std::int64_t pid = order[idx + k];
                p.hogar_id[pid] = static_cast<std::int32_t>(hid);
                ingreso += p.ingreso_laboral[pid];
                if (p.edad[pid] < 18) ++menores;
            }

            // tipo de hogar
            TipoHogar tipo;
            if (s == 1)            tipo = TipoHogar::Unipersonal;
            else if (s >= 6)       tipo = TipoHogar::Extenso;
            else if (menores > 0)  tipo = TipoHogar::Nuclear;
            else                   tipo = TipoHogar::Compuesto;

            // estrato y SISBÉN según ingreso per cápita (en SMLV)
            const double pc = (s > 0) ? ingreso / s : 0.0;
            const double r = pc / SMLV;
            std::uint8_t estrato = r < 0.5 ? 1 : r < 1.0 ? 2 : r < 1.8 ? 3 : r < 3.5 ? 4 : r < 7.0 ? 5 : 6;
            GrupoSisben sisben = r < 0.5 ? GrupoSisben::A : r < 1.0 ? GrupoSisben::B
                               : r < 2.0 ? GrupoSisben::C : GrupoSisben::D;

            double tu = U(rng);
            TenenciaVivienda ten = tu < 0.50 ? TenenciaVivienda::Propia
                                 : tu < 0.90 ? TenenciaVivienda::Arriendo : TenenciaVivienda::Otra;

            h.municipio_id.push_back(p.municipio_id[order[idx]]);
            h.departamento_id.push_back(depto);
            h.tamano.push_back(static_cast<std::uint8_t>(s));
            h.num_menores.push_back(menores);
            h.tipo.push_back(tipo);
            h.ingreso.push_back(ingreso);
            h.estrato.push_back(estrato);
            h.sisben.push_back(sisben);
            h.tenencia.push_back(ten);

            // privaciones materiales (IPM) por estrato × zona
            const int e = estrato - 1;
            const bool urbano = (p.municipio_id[order[idx]] < g.mpio_urbano.size())
                                && g.mpio_urbano[p.municipio_id[order[idx]]] >= 0.5f;
            auto priv = [&](int k){ double pr = urbano ? PRIV[k].urb[e] : PRIV[k].rur[e];
                                    return static_cast<std::uint8_t>(U(rng) < pr ? 1 : 0); };
            h.sin_agua.push_back(priv(0));
            h.sin_excretas.push_back(priv(1));
            h.piso_inadec.push_back(priv(2));
            h.pared_inadec.push_back(priv(3));
            // hacinamiento: por tasa, o forzado si >3 personas por (cuartos≈tamaño/2)
            std::uint8_t hac = priv(4); if (s >= 6 && U(rng) < 0.5) hac = 1;
            h.hacinamiento.push_back(hac);
            h.sin_internet.push_back(priv(5));
            h.barrera_priminf.push_back(menores > 0 ? priv(6) : 0);
            h.trabajo_infantil.push_back(menores > 0 ? priv(7) : 0);
            h.barrera_salud.push_back(priv(8));

            idx += s;
        }
    }
    return h;
}

namespace metrics {

Real mean_household_size(const Households& h) {
    const std::int64_t n = h.size();
    long double s = 0.0L;
    for (std::int64_t i = 0; i < n; ++i) s += h.tamano[i];
    return n ? static_cast<Real>(s / n) : 0.0;
}

Real share_tipo(const Households& h, TipoHogar t) {
    const std::int64_t n = h.size();
    std::int64_t c = 0;
    for (std::int64_t i = 0; i < n; ++i) c += (h.tipo[i] == t);
    return n ? static_cast<Real>(c) / n : 0.0;
}

Real mean_estrato(const Households& h) {
    const std::int64_t n = h.size();
    long double s = 0.0L;
    for (std::int64_t i = 0; i < n; ++i) s += h.estrato[i];
    return n ? static_cast<Real>(s / n) : 0.0;
}

Real gini_household_income(const Households& h) {
    std::vector<float> w(h.ingreso);
    std::sort(w.begin(), w.end());
    const std::int64_t n = static_cast<std::int64_t>(w.size());
    long double weighted = 0.0L, total = 0.0L;
    for (std::int64_t i = 0; i < n; ++i) { weighted += static_cast<long double>(i + 1) * w[i]; total += w[i]; }
    if (total <= 0.0L) return 0.0;
    return static_cast<Real>((2.0L * weighted) / (n * total) - static_cast<long double>(n + 1) / n);
}

} // namespace metrics
} // namespace socium
