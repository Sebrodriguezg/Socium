#include "socium/population.hpp"
#include "socium/rng.hpp"

#include <algorithm>
#include <cmath>
#include <random>

namespace socium {

void Population::resize(std::int64_t n) {
    const std::size_t N = static_cast<std::size_t>(n);
    edad.resize(N); sexo.resize(N); etnia.resize(N); estado_civil.resize(N);
    nivel_educativo.resize(N); anios_escolaridad.resize(N); asiste_escuela.resize(N);
    situacion_laboral.resize(N); sector.resize(N); posicion.resize(N);
    horas_trabajadas.resize(N); informal.resize(N); cotiza_pension.resize(N);
    ingreso_laboral.resize(N); riqueza.resize(N); afiliacion_salud.resize(N);
    confianza.resize(N); participa_org.resize(N); religiosidad.resize(N);
    opinion_politica.resize(N); satisfaccion_vida.resize(N); estatus_migratorio.resize(N);
    hogar_id.resize(N); municipio_id.resize(N); departamento_id.resize(N);
}

std::size_t Population::bytes_per_agent() {
    // 17 campos uint8 + 1 int8 + 2 float + 1 int32 + 1 uint16 (aprox; sin overhead)
    return 19 * 1 + 2 * 4 + 4 + 2;
}

// NOTA: distribuciones aproximadas (placeholder). Se reemplazarán por calibración
// con Censo 2018, GEIH e IPM por municipio. Sirven para desarrollar y medir ya.
Population build_synthetic(std::int64_t n, std::uint64_t seed) {
    Population p;
    p.resize(n);

    #pragma omp parallel
    { seed_thread_rng(seed); }

    #pragma omp parallel for schedule(static)
    for (std::int64_t i = 0; i < n; ++i) {
        auto& rng = thread_rng();
        std::uniform_real_distribution<double> U(0.0, 1.0);

        // Edad: pirámide joven (media ~31). Aproximación con distribución triangular.
        int edad = static_cast<int>(80.0 * std::pow(U(rng), 1.6));
        p.edad[i] = static_cast<std::uint8_t>(std::min(edad, 100));

        p.sexo[i] = (U(rng) < 0.51) ? Sexo::Mujer : Sexo::Hombre;

        // Etnia ~ 4.4% indígena, 9.3% NARP, resto ninguna.
        double e = U(rng);
        p.etnia[i] = e < 0.044 ? Etnia::Indigena : (e < 0.137 ? Etnia::NARP : Etnia::Ninguna);

        // Educación correlacionada con edad (los más jóvenes con más años).
        double edu = U(rng);
        NivelEducativo niv;
        if (edad < 6)       niv = NivelEducativo::Ninguno;
        else if (edu < 0.10) niv = NivelEducativo::Ninguno;
        else if (edu < 0.40) niv = NivelEducativo::Primaria;
        else if (edu < 0.70) niv = NivelEducativo::Secundaria;
        else if (edu < 0.85) niv = NivelEducativo::Media;
        else if (edu < 0.93) niv = NivelEducativo::Tecnico;
        else if (edu < 0.99) niv = NivelEducativo::Universitario;
        else                 niv = NivelEducativo::Posgrado;
        p.nivel_educativo[i] = niv;
        p.anios_escolaridad[i] = static_cast<std::uint8_t>(std::min(edad, static_cast<int>(niv) * 2 + 2));
        p.asiste_escuela[i] = (edad >= 5 && edad <= 24 && U(rng) < 0.75) ? 1 : 0;

        p.estado_civil[i] = edad < 18 ? EstadoCivil::Soltero
                          : (U(rng) < 0.4 ? EstadoCivil::UnionLibre
                          : (U(rng) < 0.5 ? EstadoCivil::Casado : EstadoCivil::Soltero));

        // Mercado laboral.
        SituacionLaboral sit;
        if (edad < 15)                 sit = SituacionLaboral::MenorEdad;
        else if (edad > 65)            sit = (U(rng) < 0.25) ? SituacionLaboral::Ocupado : SituacionLaboral::Inactivo;
        else {
            double r = U(rng);
            sit = r < 0.60 ? SituacionLaboral::Ocupado
                : (r < 0.66 ? SituacionLaboral::Desocupado : SituacionLaboral::Inactivo);
        }
        p.situacion_laboral[i] = sit;

        if (sit == SituacionLaboral::Ocupado) {
            int s = 1 + static_cast<int>(U(rng) * 7);             // 1..7
            p.sector[i] = static_cast<Sector>(std::min(s, 7));
            double pos = U(rng);
            p.posicion[i] = pos < 0.45 ? PosicionOcupacional::Asalariado
                          : (pos < 0.85 ? PosicionOcupacional::CuentaPropia
                          : (pos < 0.92 ? PosicionOcupacional::Empleador
                          : PosicionOcupacional::SinRemuneracion));
            // Informalidad ~58% (clave en Colombia), mayor en cuenta propia.
            double pinf = (p.posicion[i] == PosicionOcupacional::CuentaPropia) ? 0.85 : 0.40;
            p.informal[i] = (U(rng) < pinf) ? 1 : 0;
            p.cotiza_pension[i] = p.informal[i] ? (U(rng) < 0.1 ? 1 : 0) : (U(rng) < 0.8 ? 1 : 0);
            p.horas_trabajadas[i] = static_cast<std::uint8_t>(30 + U(rng) * 30);
            // Ingreso lognormal correlacionado con educación (SMLV 2024 ~1.3M COP).
            double mu = 13.6 + 0.18 * static_cast<int>(niv);     // ln(COP)
            std::lognormal_distribution<double> LN(mu, 0.7);
            p.ingreso_laboral[i] = static_cast<float>(LN(rng));
        } else {
            p.sector[i] = Sector::NoAplica;
            p.posicion[i] = PosicionOcupacional::NoAplica;
            p.informal[i] = 0; p.cotiza_pension[i] = 0;
            p.horas_trabajadas[i] = 0; p.ingreso_laboral[i] = 0.0f;
        }

        p.riqueza[i] = static_cast<float>(p.ingreso_laboral[i] * (6.0 + U(rng) * 24.0));

        // Salud: cobertura ~95% (contributivo vs subsidiado según informalidad).
        double salud = U(rng);
        p.afiliacion_salud[i] = salud < 0.05 ? AfiliacionSalud::Ninguno
                              : (p.informal[i] ? AfiliacionSalud::Subsidiado
                              : (salud < 0.55 ? AfiliacionSalud::Contributivo : AfiliacionSalud::Subsidiado));

        // Social / político.
        p.confianza[i] = static_cast<std::uint8_t>(U(rng) * 40);  // baja confianza (CO ~bajo)
        p.participa_org[i] = (U(rng) < 0.2) ? 1 : 0;
        double rel = U(rng);
        p.religiosidad[i] = rel < 0.6 ? Religiosidad::Catolica
                          : (rel < 0.8 ? Religiosidad::Cristiana
                          : (rel < 0.9 ? Religiosidad::Ninguna : Religiosidad::Otra));
        p.opinion_politica[i] = static_cast<std::int8_t>((U(rng) - 0.5) * 200);
        p.satisfaccion_vida[i] = static_cast<std::uint8_t>(4 + U(rng) * 6);

        // Migración ~ 5% venezolanos.
        double mig = U(rng);
        p.estatus_migratorio[i] = mig < 0.05
            ? (U(rng) < 0.6 ? EstatusMigratorio::MigranteRegular : EstatusMigratorio::MigranteIrregular)
            : EstatusMigratorio::Nacional;

        // Geografía (placeholder uniforme; se reemplaza por DIVIPOLA real).
        p.departamento_id[i] = static_cast<std::uint8_t>(U(rng) * 33);
        p.municipio_id[i] = static_cast<std::uint16_t>(U(rng) * 1103);
        p.hogar_id[i] = static_cast<std::int32_t>(i / 3);          // ~3 personas/hogar
    }
    return p;
}

namespace metrics {

Real mean_age(const Population& p) {
    const std::int64_t n = p.size();
    long double s = 0.0L;
    #pragma omp parallel for reduction(+:s) schedule(static)
    for (std::int64_t i = 0; i < n; ++i) s += p.edad[i];
    return n ? static_cast<Real>(s / n) : 0.0;
}

Real share_female(const Population& p) {
    const std::int64_t n = p.size();
    std::int64_t f = 0;
    #pragma omp parallel for reduction(+:f) schedule(static)
    for (std::int64_t i = 0; i < n; ++i) f += (p.sexo[i] == Sexo::Mujer);
    return n ? static_cast<Real>(f) / n : 0.0;
}

Real unemployment_rate(const Population& p) {
    const std::int64_t n = p.size();
    std::int64_t ocup = 0, desoc = 0;
    #pragma omp parallel for reduction(+:ocup,desoc) schedule(static)
    for (std::int64_t i = 0; i < n; ++i) {
        ocup  += (p.situacion_laboral[i] == SituacionLaboral::Ocupado);
        desoc += (p.situacion_laboral[i] == SituacionLaboral::Desocupado);
    }
    const std::int64_t pea = ocup + desoc;
    return pea ? static_cast<Real>(desoc) / pea : 0.0;
}

Real informality_rate(const Population& p) {
    const std::int64_t n = p.size();
    std::int64_t ocup = 0, inf = 0;
    #pragma omp parallel for reduction(+:ocup,inf) schedule(static)
    for (std::int64_t i = 0; i < n; ++i) {
        if (p.situacion_laboral[i] == SituacionLaboral::Ocupado) {
            ++ocup; inf += p.informal[i];
        }
    }
    return ocup ? static_cast<Real>(inf) / ocup : 0.0;
}

Real mean_labor_income(const Population& p) {
    const std::int64_t n = p.size();
    long double s = 0.0L; std::int64_t ocup = 0;
    #pragma omp parallel for reduction(+:s,ocup) schedule(static)
    for (std::int64_t i = 0; i < n; ++i)
        if (p.situacion_laboral[i] == SituacionLaboral::Ocupado) { s += p.ingreso_laboral[i]; ++ocup; }
    return ocup ? static_cast<Real>(s / ocup) : 0.0;
}

Real gini_income(const Population& p) {
    std::vector<float> w;
    w.reserve(p.size());
    for (std::int64_t i = 0; i < p.size(); ++i)
        if (p.situacion_laboral[i] == SituacionLaboral::Ocupado) w.push_back(p.ingreso_laboral[i]);
    if (w.empty()) return 0.0;
    std::sort(w.begin(), w.end());
    const std::int64_t m = static_cast<std::int64_t>(w.size());
    long double weighted = 0.0L, total = 0.0L;
    for (std::int64_t i = 0; i < m; ++i) { weighted += static_cast<long double>(i + 1) * w[i]; total += w[i]; }
    if (total <= 0.0L) return 0.0;
    return static_cast<Real>((2.0L * weighted) / (m * total) - static_cast<long double>(m + 1) / m);
}

} // namespace metrics
} // namespace socium
