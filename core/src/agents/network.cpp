#include "socium/network.hpp"
#include "socium/population.hpp"
#include "socium/rng.hpp"

#include <algorithm>
#include <vector>

namespace socium {

// counting sort: devuelve offsets (size G+1) y order (size N) agrupando por clave key[i]∈[0,G)
template <class Key>
static void agrupar(const std::vector<Key>& key, std::int64_t N, std::int64_t G,
                    std::vector<std::int64_t>& off, std::vector<std::int64_t>& order) {
    off.assign(static_cast<std::size_t>(G) + 1, 0);
    for (std::int64_t i = 0; i < N; ++i) off[key[i] + 1]++;
    for (std::int64_t g = 0; g < G; ++g) off[g + 1] += off[g];
    order.resize(static_cast<std::size_t>(N));
    std::vector<std::int64_t> cur(off.begin(), off.end() - 1);
    for (std::int64_t i = 0; i < N; ++i) order[cur[key[i]]++] = i;
}

Network construir_red(const Population& p, int grado_geo, std::uint64_t seed) {
    const std::int64_t N = p.size();
    Network net;
    if (N == 0) return net;

    // agrupar por municipio y por hogar
    std::int64_t M = 0, H = 0;
    for (std::int64_t i = 0; i < N; ++i) { M = std::max<std::int64_t>(M, p.municipio_id[i] + 1);
                                           H = std::max<std::int64_t>(H, p.hogar_id[i] + 1); }
    std::vector<std::int64_t> offM, ordM, offH, ordH;
    agrupar(p.municipio_id, N, M, offM, ordM);
    agrupar(p.hogar_id, N, H, offH, ordH);

    // grados: (tamaño hogar - 1) + grado_geo
    net.off.assign(static_cast<std::size_t>(N) + 1, 0);
    for (std::int64_t i = 0; i < N; ++i) {
        const std::int32_t h = p.hogar_id[i];
        const std::int64_t hsz = offH[h + 1] - offH[h];
        net.off[i + 1] = net.off[i] + std::max<std::int64_t>(0, hsz - 1) + grado_geo;
    }
    net.nbr.resize(static_cast<std::size_t>(net.off[N]));

    #pragma omp parallel
    { seed_thread_rng(seed); }

    #pragma omp parallel for schedule(static)
    for (std::int64_t i = 0; i < N; ++i) {
        std::int64_t pos = net.off[i];
        // capa hogar: co-miembros
        const std::int32_t h = p.hogar_id[i];
        for (std::int64_t k = offH[h]; k < offH[h + 1]; ++k) {
            const std::int64_t j = ordH[k];
            if (j != i) net.nbr[pos++] = static_cast<std::int32_t>(j);
        }
        // capa comunidad: grado_geo vecinos aleatorios del mismo municipio
        const std::int64_t b = offM[p.municipio_id[i]], e = offM[p.municipio_id[i] + 1], L = e - b;
        auto& rng = thread_rng();
        for (int g = 0; g < grado_geo; ++g) {
            std::int64_t j = i;
            if (L > 1) { std::uniform_int_distribution<std::int64_t> d(0, L - 1); j = ordM[b + d(rng)]; }
            net.nbr[pos++] = static_cast<std::int32_t>(j);
        }
    }
    return net;
}

} // namespace socium
