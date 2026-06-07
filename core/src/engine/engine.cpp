#include "socium/engine.hpp"
#include "socium/society.hpp"
#include "socium/network.hpp"
#include "socium/rng.hpp"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

namespace socium {

// --- helpers ---
static inline bool nivel_bajo(NivelEducativo n) {
    return static_cast<int>(n) <= static_cast<int>(NivelEducativo::Primaria);
}
static NivelEducativo nivel_por_anios(int a) {
    if (a <= 0)  return NivelEducativo::Ninguno;
    if (a <= 5)  return NivelEducativo::Primaria;
    if (a <= 9)  return NivelEducativo::Secundaria;
    if (a <= 11) return NivelEducativo::Media;
    if (a <= 13) return NivelEducativo::Tecnico;
    if (a <= 16) return NivelEducativo::Universitario;
    return NivelEducativo::Posgrado;
}
// mortalidad anual por edad (Gompertz aprox.; calibrar con tablas de vida DANE).
// ~0.55% crudo con esta estructura etaria y esperanza de vida ~78.
static double p_morir(int edad) {
    if (edad < 1) return 0.012;
    return std::min(0.00003 * std::exp(0.09 * edad), 1.0);
}

Politicas Politicas::load(const std::string& path) {
    Politicas pol;
    std::ifstream in(path);
    if (!in) return pol;
    std::string line;
    while (std::getline(in, line)) {
        if (line.empty() || line[0] == '#') continue;
        std::istringstream ss(line);
        std::string k; double v;
        if (!(ss >> k >> v)) continue;
        if      (k == "transfer_ingreso_pc")  pol.transfer_ingreso_pc = v;
        else if (k == "desercion_mult")        pol.desercion_mult = v;
        else if (k == "salud_prob_mult")       pol.salud_prob_mult = v;
        else if (k == "aseguramiento_boost")   pol.aseguramiento_boost = v;
        else if (k == "crimen_base_mult")      pol.crimen_base_mult = v;
        else if (k == "crimen_abandono_mult")  pol.crimen_abandono_mult = v;
        else if (k == "smlv_mult")             pol.smlv_mult = v;
        else if (k == "empleo_mult")           pol.empleo_mult = v;
        else if (k == "impuesto_mult")         pol.impuesto_mult = v;
        else if (k == "gasto_mult")            pol.gasto_mult = v;
    }
    return pol;
}

Engine::Engine(Population& p, Households& h, Firms& f, const Geography& g, Parametros par,
               EngineConfig cfg, Politicas pol)
    : p_(p), h_(h), f_(f), g_(g), par_(par), cfg_(cfg), pol_(pol) {
    #pragma omp parallel
    { seed_thread_rng(cfg_.seed); }
    // política de aseguramiento: afiliar a una fracción de los sin afiliación (M4)
    if (pol_.aseguramiento_boost > 0.0) {
        const std::int64_t N = p_.size();
        for (std::int64_t i = 0; i < N; ++i)
            if (p_.afiliacion_salud[i] == AfiliacionSalud::Ninguno && uniform01() < pol_.aseguramiento_boost)
                p_.afiliacion_salud[i] = AfiliacionSalud::Subsidiado;
    }
    std::int32_t mh = 0;
    for (std::int64_t i = 0; i < p_.size(); ++i) mh = std::max(mh, p_.hogar_id[i] + 1);
    next_hogar_id_ = mh;
    recomputar_ingreso_hogar();
}

// ingreso per cápita del hogar para cada persona (M1/M5 pobreza)
void Engine::recomputar_ingreso_hogar() {
    const std::int64_t N = p_.size();
    std::int64_t maxh = 0;
    for (std::int64_t i = 0; i < N; ++i) maxh = std::max<std::int64_t>(maxh, p_.hogar_id[i] + 1);
    std::vector<double> suma(maxh, 0.0);
    std::vector<int>    cnt(maxh, 0);
    for (std::int64_t i = 0; i < N; ++i) {
        if (!p_.vivo[i]) continue;
        // ingreso laboral + ingresos no laborales (pensiones, subsidios, rebusque)
        double ing = p_.ingreso_laboral[i];
        if (p_.edad[i] >= 65) {
            ing += (static_cast<int>(p_.nivel_educativo[i]) >= static_cast<int>(NivelEducativo::Media))
                   ? par_.pension_contributiva : par_.colombia_mayor;
        } else if (p_.edad[i] >= 18 && p_.situacion_laboral[i] != SituacionLaboral::Ocupado) {
            ing += par_.subsistencia_informal;   // 'rebusque' informal de no ocupados
        }
        suma[p_.hogar_id[i]] += ing;
        cnt[p_.hogar_id[i]]++;
    }
    hh_pc_.assign(N, 0.0f);
    for (std::int64_t i = 0; i < N; ++i) {
        const std::int32_t hid = p_.hogar_id[i];
        float pc = cnt[hid] > 0 ? static_cast<float>(suma[hid] / cnt[hid]) : 0.0f;
        // piso no laboral del hogar (calibración GEIH) + transferencias (política)
        hh_pc_[i] = pc + static_cast<float>(par_.ingreso_no_laboral_pc + pol_.transfer_ingreso_pc);
    }
}

// M7 — envejecer, morir, nacer
void Engine::demografia() {
    const std::int64_t N = p_.size();
    // envejecer + mortalidad
    #pragma omp parallel for schedule(static)
    for (std::int64_t i = 0; i < N; ++i) {
        if (!p_.vivo[i]) continue;
        if (p_.edad[i] < 110) p_.edad[i]++;
        double pm = p_morir(p_.edad[i]);
        if (p_.meses_enfermo[i] > 0) pm *= 1.5;        // enfermedad sube mortalidad
        if (uniform01() < pm) p_.vivo[i] = 0;
    }
    // nacimientos (serial: hace push_back) — tasa por mujer fértil = TFR/35
    const double tasa = par_.fecundidad_total / 35.0;
    for (std::int64_t i = 0; i < N; ++i) {
        if (!p_.vivo[i] || p_.sexo[i] != Sexo::Mujer) continue;
        if (p_.edad[i] < 15 || p_.edad[i] > 49) continue;
        if (uniform01() < tasa) {
            Sexo s = (uniform01() < 0.51) ? Sexo::Mujer : Sexo::Hombre;
            p_.nacer(s, p_.hogar_id[i], p_.municipio_id[i], p_.departamento_id[i]);
        }
    }
}

// M1 — trayectoria educativa (continuar vs desertar)
void Engine::educacion() {
    const std::int64_t N = p_.size();
    const double e1 = par_.desercion_estrato1, e6 = par_.desercion_estrato6;
    #pragma omp parallel for schedule(static)
    for (std::int64_t i = 0; i < N; ++i) {
        if (!p_.vivo[i]) continue;
        const int edad = p_.edad[i];
        if (edad == 6 && !p_.asiste_escuela[i]) p_.asiste_escuela[i] = 1; // entrada a la escuela
        if (edad < 6 || edad > 24) continue;
        if (!p_.asiste_escuela[i]) continue;

        // p_desercion según estrato del hogar (M1)
        int estrato = 3;
        const std::int32_t hid = p_.hogar_id[i];
        if (hid >= 0 && hid < h_.size()) estrato = h_.estrato[hid];
        double pdes;
        if (edad <= 17) {
            // edad escolar: base MEN (~3.7%) escalada por estrato (más pobre, más deserción)
            pdes = par_.desercion_base_5_16 * (1.6 - 0.12 * (estrato - 1));
        } else {
            // educación superior (18-24): tasas LEE Javeriana por estrato
            pdes = e1 + (e6 - e1) * (estrato - 1) / 5.0;
        }
        pdes *= pol_.desercion_mult;   // política educativa (M1)

        if (uniform01() < pdes) {
            p_.asiste_escuela[i] = 0;                  // deserta
        } else {
            if (p_.anios_escolaridad[i] < 18) p_.anios_escolaridad[i]++;
            p_.nivel_educativo[i] = nivel_por_anios(p_.anios_escolaridad[i]);
        }
    }
}

// participación laboral (probabilidad) por agente — oferta de trabajo (no es palanca)
static inline double prob_participacion(const Population& p, std::int64_t i) {
    double pp = 0.55 + 0.03 * static_cast<int>(p.nivel_educativo[i]);
    if (p.edad[i] > 65) pp *= 0.4;
    return std::min(1.0, pp);
}

// M2/M3 — empleo ENDÓGENO a la capacidad de las empresas. El emparejamiento se hace a
// nivel DEPARTAMENTAL (mercados laborales regionales): 33 grupos siempre bien muestreados,
// evita el artefacto de saturación de municipios chicos (desempleo invariante de escala).
void Engine::mercado_laboral() {
    const std::int64_t N = p_.size();
    const std::int64_t D = g_.n_departamentos();

    // 1) puestos de trabajo por departamento (empresas activas) modulados por el ciclo
    std::vector<double> slots(static_cast<std::size_t>(D), 0.0);
    for (std::int64_t k = 0; k < f_.size(); ++k)
        if (f_.activa[k]) slots[f_.departamento[k]] += f_.empleos[k];
    // ciclo económico + política de empleo/formalización (palanca = demanda de trabajo)
    // shock exógeno: golpea el empleo de forma moderada (la mayor parte va vía ingreso)
    const double shock_empleo = 1.0 - 0.35 * (1.0 - shock_actual_);
    for (std::int64_t d = 0; d < D; ++d) slots[d] *= ciclo_ * pol_.empleo_mult * shock_empleo;

    // 2) participantes esperados por departamento (suma de probabilidades de participación)
    std::vector<double> exppart(static_cast<std::size_t>(D), 0.0);
    for (std::int64_t i = 0; i < N; ++i) {
        if (!p_.vivo[i] || p_.edad[i] < 15 || p_.asiste_escuela[i]) continue;
        exppart[p_.departamento_id[i]] += prob_participacion(p_, i);
    }
    // 3) tasa de ocupación por departamento = min(1, puestos / participantes)
    std::vector<double> occ(static_cast<std::size_t>(D), 0.0);
    for (std::int64_t d = 0; d < D; ++d)
        occ[d] = std::min(1.0, slots[d] / std::max(1.0, exppart[d]));

    #pragma omp parallel for schedule(static)
    for (std::int64_t i = 0; i < N; ++i) {
        if (!p_.vivo[i]) continue;
        const int edad = p_.edad[i];
        if (edad < 15) { p_.situacion_laboral[i] = SituacionLaboral::MenorEdad; p_.ingreso_laboral[i] = 0; continue; }
        if (p_.asiste_escuela[i]) { p_.situacion_laboral[i] = SituacionLaboral::Inactivo; p_.ingreso_laboral[i] = 0; continue; }

        const int anios = p_.anios_escolaridad[i];
        if (uniform01() > prob_participacion(p_, i)) {
            p_.situacion_laboral[i] = SituacionLaboral::Inactivo; p_.ingreso_laboral[i] = 0; continue;
        }
        // ¿consigue uno de los puestos del municipio? (sesgo por educación, centrado ~1)
        const double educ_factor = 0.75 + 0.10 * static_cast<int>(p_.nivel_educativo[i]);
        if (uniform01() > occ[p_.departamento_id[i]] * educ_factor) {
            p_.situacion_laboral[i] = SituacionLaboral::Desocupado; p_.ingreso_laboral[i] = 0; continue;
        }

        // ocupado: ingreso Mincer + residual lognormal (dispersión salarial real;
        // sigma alto reproduce Gini ~0.5 y cola de Pareto, spec §1.4/§7.3)
        p_.situacion_laboral[i] = SituacionLaboral::Ocupado;
        const int exper = std::max(0, edad - anios - 6);
        std::normal_distribution<double> ruido(0.0, par_.sigma_ingreso);   // residual de Mincer (Gini)
        double ln = par_.retorno_anual_escolaridad * (anios - 11) + 0.03 * exper
                  - 0.0004 * exper * exper + ruido(thread_rng());
        // productividad regional: ingreso escalado por el PIB pc del departamento (spec §5.2)
        const std::uint8_t dep = p_.departamento_id[i];
        double prod_dep = (dep < g_.dpto_productividad.size()) ? g_.dpto_productividad[dep] : 1.0;
        double f_dep = std::pow(std::max(0.1, prod_dep), par_.elasticidad_productividad);
        double ingreso = par_.smlv * par_.calib_ingreso * pol_.smlv_mult * productividad_ * f_dep * shock_actual_ * std::exp(ln);
        // informalidad (M3): por urbano/rural real del municipio (spec §2.4:
        // urbano 43%, rural 84.7%) modulada por educación
        const std::uint16_t mi = p_.municipio_id[i];
        double urb = (mi < g_.mpio_urbano.size()) ? g_.mpio_urbano[mi] : 0.7;
        double p_inf = (0.847 - 0.417 * urb) * (anios < 11 ? 1.15 : 0.75);
        p_inf = std::min(0.95, std::max(0.05, p_inf));
        p_.informal[i] = (uniform01() < p_inf) ? 1 : 0;
        if (p_.informal[i]) ingreso *= (1.0 - par_.penalizacion_informalidad);
        if (p_.sexo[i] == Sexo::Mujer) ingreso *= (1.0 - par_.brecha_genero);
        if (p_.es_delincuente[i]) ingreso *= 0.7;     // menor inserción laboral formal
        p_.ingreso_laboral[i] = static_cast<float>(std::max(ingreso, par_.smlv * 0.15));
    }
}

// M4 — salud: enfermar/recuperar y efecto en riqueza (gasto/ingreso perdido)
void Engine::salud_mensual() {
    const std::int64_t N = p_.size();
    #pragma omp parallel for schedule(static)
    for (std::int64_t i = 0; i < N; ++i) {
        if (!p_.vivo[i]) continue;
        if (p_.meses_enfermo[i] > 0) { p_.meses_enfermo[i]--; continue; } // recuperándose
        double pe = par_.prob_base_enfermar_mensual * pol_.salud_prob_mult;
        if (p_.afiliacion_salud[i] == AfiliacionSalud::Ninguno) pe *= par_.rr_enfermar_sin_aseguramiento;
        if (hh_pc_[i] < static_cast<float>(par_.linea_pobreza_mensual)) pe *= 1.3;
        if (p_.edad[i] > 60 || p_.edad[i] < 5) pe *= 1.5;
        if (uniform01() < pe) {
            p_.meses_enfermo[i] = static_cast<std::uint8_t>(1 + static_cast<int>(uniform01() * 3));
            // pérdida económica: gasto de bolsillo + ingreso no percibido
            p_.riqueza[i] -= static_cast<float>(par_.perdida_productividad_enfermo * p_.ingreso_laboral[i]);
            if (p_.riqueza[i] < 0) p_.riqueza[i] = 0;
        }
    }
}

// M5 — emergencia/abandono de la delincuencia (factores de riesgo)
void Engine::delincuencia() {
    const std::int64_t N = p_.size();
    // snapshot del estado de delincuencia para el efecto de pares (evita carreras)
    std::vector<std::uint8_t> prev;
    if (red_) prev = p_.es_delincuente;
    #pragma omp parallel for schedule(static)
    for (std::int64_t i = 0; i < N; ++i) {
        if (!p_.vivo[i]) continue;
        const int edad = p_.edad[i];
        if (edad < 12 || edad > 50) continue;

        const bool nini = (edad >= 15 && edad <= 28 && !p_.asiste_escuela[i]
                           && p_.situacion_laboral[i] != SituacionLaboral::Ocupado);
        const bool pobre = hh_pc_[i] < static_cast<float>(par_.linea_pobreza_mensual);
        const bool bajaeduc = nivel_bajo(static_cast<NivelEducativo>(p_.nivel_educativo[i]));
        const bool hombrejoven = (p_.sexo[i] == Sexo::Hombre && edad >= 15 && edad <= 29);

        double pr = par_.crimen_prob_base * pol_.crimen_base_mult;
        if (nini) pr *= par_.rr_nini;
        if (pobre) pr *= par_.rr_pobreza;
        if (bajaeduc) pr *= par_.rr_baja_educacion;
        if (hombrejoven) pr *= par_.rr_hombre_joven;
        // presencia de grupos armados / conflicto local (spec §6.4 β₅): índice [0,1]
        const std::uint16_t mi = p_.municipio_id[i];
        if (mi < g_.mpio_conflicto.size())
            pr *= 1.0 + (par_.rr_presencia_gao - 1.0) * g_.mpio_conflicto[mi];
        // efecto de pares (red social): fracción de vecinos que delinquen
        if (red_ && i < red_->n()) {
            const std::int64_t a = red_->off[i], b = red_->off[i + 1];
            if (b > a) {
                std::int64_t d = 0;
                for (std::int64_t e = a; e < b; ++e) d += prev[red_->nbr[e]];
                pr *= 1.0 + (par_.rr_peer_delito - 1.0) * (static_cast<double>(d) / (b - a));
            }
        }

        if (!p_.es_delincuente[i]) {
            if (uniform01() < pr) p_.es_delincuente[i] = 1;
        } else {
            // abandono: mayor si ahora estudia o trabaja (oportunidades)
            double pa = par_.prob_abandono_delito * pol_.crimen_abandono_mult;
            if (p_.asiste_escuela[i] || p_.situacion_laboral[i] == SituacionLaboral::Ocupado) pa *= 2.0;
            if (uniform01() < pa) p_.es_delincuente[i] = 0;
        }
    }
}

// M-migración interna por gravedad (spec §6.3): los agentes huyen del conflicto del
// origen hacia destinos grandes y seguros. Destino ~ pop^alpha · (1 - conflicto).
void Engine::migracion() {
    migraciones_anio_ = 0;
    const std::int64_t M = g_.n_municipios();
    if (M == 0 || static_cast<std::int64_t>(g_.mpio_peso.size()) != M) return;

    std::vector<double> cum(static_cast<std::size_t>(M));
    double acc = 0.0;
    for (std::int64_t m = 0; m < M; ++m) {
        const double pop = g_.mpio_peso[m];
        const double conf = (m < static_cast<std::int64_t>(g_.mpio_conflicto.size())) ? g_.mpio_conflicto[m] : 0.0;
        acc += std::pow(std::max(pop, 1.0), par_.gravity_alpha) * (1.0 - 0.7 * conf);
        cum[m] = acc;
    }
    if (acc <= 0.0) return;

    auto& rng = thread_rng();
    std::uniform_real_distribution<double> U(0.0, 1.0);
    const std::int64_t N = p_.size();
    for (std::int64_t i = 0; i < N; ++i) {
        if (!p_.vivo[i]) continue;
        const std::uint16_t orig = p_.municipio_id[i];
        const double conf = (orig < g_.mpio_conflicto.size()) ? g_.mpio_conflicto[orig] : 0.0;
        double pmig = par_.migracion_base * (1.0 + par_.push_conflicto * conf);
        if (p_.situacion_laboral[i] == SituacionLaboral::Desocupado) pmig *= 1.5;
        if (U(rng) >= pmig) continue;

        std::int64_t j = std::lower_bound(cum.begin(), cum.end(), U(rng) * acc) - cum.begin();
        if (j >= M) j = M - 1;
        if (static_cast<std::uint16_t>(j) == orig) continue;       // no migra al mismo sitio
        p_.municipio_id[i]    = static_cast<std::uint16_t>(j);
        p_.departamento_id[i] = g_.mpio_dpto[static_cast<std::size_t>(j)];
        p_.hogar_id[i]        = next_hogar_id_++;                   // nuevo hogar en destino
        ++migraciones_anio_;
    }
}

// Sector financiero: crédito de hogares. Los hogares cortos de ingreso piden prestado
// (informal 'gota a gota' al ~200% si son pobres/informales, formal si no); el excedente
// repaga. La deuda alta = estrés financiero (trampa de deuda). No altera la pobreza
// monetaria (def. DANE por ingreso); es una dimensión de vulnerabilidad aparte.
void Engine::finanzas() {
    const std::int64_t N = p_.size();
    const double linea = par_.linea_pobreza_mensual;
    #pragma omp parallel for schedule(static)
    for (std::int64_t i = 0; i < N; ++i) {
        if (!p_.vivo[i] || p_.edad[i] < 18) continue;
        const double pc = hh_pc_[i];
        const bool informal_credito = p_.informal[i] || pc < 1.5 * linea;
        const double tasa = informal_credito ? par_.tasa_gota_gota : par_.tasa_credito_formal;
        double d = p_.deuda[i] * (1.0 + tasa);                 // interés
        if (pc < linea) d += (linea - pc) * 6.0;               // pide prestado para subsistir
        else            d = std::max(0.0, d - (pc - linea) * 3.0);  // repaga con excedente
        const double techo = std::max(linea, pc) * 12.0 * 3.0; // techo de endeudamiento
        p_.deuda[i] = static_cast<float>(std::min(d, techo));
    }
}

// M-opinión / bienestar subjetivo / radicalización (spec §6.6).
// Actualiza la satisfacción con la vida (0-10) según condiciones reales del agente y
// mueve la opinión política por conformidad con el municipio + estrés económico (que
// empuja a los extremos = polarización/radicalización).
void Engine::opinion() {
    const std::int64_t N = p_.size();
    const std::int64_t M = g_.n_municipios();

    // media de opinión por municipio (campo social local)
    std::vector<double> som(static_cast<std::size_t>(M), 0.0);
    std::vector<std::int64_t> cnt(static_cast<std::size_t>(M), 0);
    for (std::int64_t i = 0; i < N; ++i) {
        if (!p_.vivo[i]) continue;
        som[p_.municipio_id[i]] += p_.opinion_politica[i];
        cnt[p_.municipio_id[i]]++;
    }
    std::vector<float> media(static_cast<std::size_t>(M), 0.0f);
    for (std::int64_t m = 0; m < M; ++m) if (cnt[m]) media[m] = static_cast<float>(som[m] / cnt[m]);

    std::vector<std::int8_t> prev_op;   // snapshot para conformidad por red
    if (red_) prev_op = p_.opinion_politica;

    const float linea = static_cast<float>(par_.linea_pobreza_mensual);
    #pragma omp parallel for schedule(static)
    for (std::int64_t i = 0; i < N; ++i) {
        if (!p_.vivo[i]) continue;
        const std::uint16_t mi = p_.municipio_id[i];
        const double conf = (mi < g_.mpio_conflicto.size()) ? g_.mpio_conflicto[mi] : 0.0;

        // --- satisfacción con la vida (bienestar subjetivo, dominio 15) ---
        double sat = 5.0;
        sat += (hh_pc_[i] >= linea) ? 1.5 : -2.5;
        if (p_.meses_enfermo[i] > 0) sat -= 1.5;
        // estrés financiero (trampa de deuda) golpea el bienestar
        if (p_.deuda[i] > par_.umbral_estres_financiero * std::max(hh_pc_[i], 1.0f) * 12.0f) sat -= 1.0;
        if (p_.situacion_laboral[i] == SituacionLaboral::Ocupado) sat += 1.0;
        else if (p_.situacion_laboral[i] == SituacionLaboral::Desocupado) sat -= 1.0;
        sat -= par_.peso_seguridad_bienestar * conf;
        sat += 2.0 * crecimiento_;                       // el ciclo macro mejora el ánimo
        sat = std::min(10.0, std::max(0.0, sat));
        // suavizado (memoria del estado anterior)
        p_.satisfaccion_vida[i] = static_cast<std::uint8_t>(0.5 * p_.satisfaccion_vida[i] + 0.5 * sat);

        // --- opinión política: conformidad + estrés que radicaliza ---
        double op = p_.opinion_politica[i];
        double entorno = media[mi];                         // por defecto: media del municipio
        if (red_ && i < red_->n()) {                        // si hay red: media de los vecinos
            const std::int64_t a = red_->off[i], b = red_->off[i + 1];
            if (b > a) { long s = 0; for (std::int64_t e = a; e < b; ++e) s += prev_op[red_->nbr[e]];
                         entorno = static_cast<double>(s) / (b - a); }
        }
        op += par_.conformidad_social * (entorno - op);     // conformidad con el entorno
        const bool estres = (hh_pc_[i] < linea) || (p_.situacion_laboral[i] == SituacionLaboral::Desocupado) || (conf > 0.3);
        if (estres) op += (op >= 0 ? 1.0 : -1.0) * 6.0 * (0.5 + conf);  // empuja al extremo
        op = std::min(100.0, std::max(-100.0, op));
        p_.opinion_politica[i] = static_cast<std::int8_t>(op);
    }
}

// §3 — dinámica de empresas: utilidad, extorsión en zonas de conflicto, quiebra y
// entrada. Cadena clave: conflicto -> extorsión -> quiebra -> menos empleos.
// La política de seguridad (pol_.crimen_base_mult) reduce el conflicto efectivo.
void Engine::dinamica_empresas() {
    const std::int64_t F = f_.size();
    #pragma omp parallel for schedule(static)
    for (std::int64_t k = 0; k < F; ++k) {
        const std::uint16_t m = f_.municipio[k];
        double conf = (m < g_.mpio_conflicto.size()) ? g_.mpio_conflicto[m] : 0.0;
        conf = std::min(1.0, std::max(0.0, conf * pol_.crimen_base_mult));  // seguridad reduce conflicto efectivo

        if (f_.activa[k]) {
            const double bruto = f_.empleos[k] * par_.margen_trabajador_anual * ciclo_;
            const double extorsion = par_.extorsion_tasa * bruto * conf;
            const double neto = par_.margen_operativo * bruto - extorsion;
            f_.capital[k] += static_cast<float>(neto);
            if (f_.capital[k] < 0.0f) { f_.activa[k] = 0; }   // quiebra -> pierde sus empleos
            else if (neto > 0.0 && ciclo_ > 0.98 && uniform01() < 0.08 * ciclo_)
                f_.empleos[k] += 1;                            // contratación en buen ciclo
        } else {
            // entrada/reactivación (emprendimiento) en ciclo favorable y baja extorsión
            if (ciclo_ > 0.97 && uniform01() < par_.prob_entrada_empresa * (1.0 - conf)) {
                f_.activa[k] = 1;
                f_.tamano[k] = TamanoEmpresa::Micro;
                f_.empleos[k] = 1 + static_cast<int>(uniform01() * 5);
                f_.capital[k] = static_cast<float>(par_.margen_trabajador_anual);  // capital semilla
            }
        }
    }
}

void Engine::economia_mensual() {
    EconomyParams ep;
    ep.rule = cfg_.regla_economia;
    ep.lambda = cfg_.lambda;
    simular_economia(p_, ep, 2, cfg_.economia_local, cfg_.seed);
}

// §8 paso 12 — cierre micro-macro: agrega PIB y empleo, calcula crecimiento y
// realimenta el ciclo económico y la productividad para el año siguiente.
void Engine::cerrar_macro() {
    const std::int64_t N = p_.size();
    long double pib = 0.0L;
    #pragma omp parallel for reduction(+:pib) schedule(static)
    for (std::int64_t i = 0; i < N; ++i)
        if (p_.vivo[i] && p_.situacion_laboral[i] == SituacionLaboral::Ocupado)
            pib += p_.ingreso_laboral[i];

    if (prev_pib_ > 0.0) {
        crecimiento_ = static_cast<double>(pib) / prev_pib_ - 1.0;
        // ciclo: el crecimiento por encima/por debajo de la tendencia realimenta el empleo
        double objetivo = 1.0 + par_.sensibilidad_ciclo * (crecimiento_ - par_.productividad_anual);
        ciclo_ = std::min(1.25, std::max(0.8, 0.5 * ciclo_ + 0.5 * objetivo)); // suavizado
    }

    // --- cierre fiscal stock-flow consistent (anual) ---
    const double PIB = std::max(1.0, static_cast<double>(pib) * 12.0);   // proxy anual
    std::int64_t vivos = 0;
    for (std::int64_t i = 0; i < N; ++i) vivos += p_.vivo[i];
    if (deuda_ == 0.0) deuda_ = par_.deuda_inicial_pib * PIB;            // deuda inicial (año base)

    const double recaudo = par_.tasa_tributaria_pib * pol_.impuesto_mult * PIB;
    // costo fiscal SOLO de las transferencias de política (el piso no laboral es renta privada)
    const double transferencias = pol_.transfer_ingreso_pc * static_cast<double>(vivos) * 12.0;
    const double gasto = par_.gasto_social_base_pib * pol_.gasto_mult * PIB
                         + transferencias + par_.interes_deuda * deuda_;
    const double deficit = gasto - recaudo;
    deuda_ += deficit;
    recaudo_pib_ = recaudo / PIB;
    deficit_pib_ = deficit / PIB;
    deuda_pib_   = deuda_ / PIB;
    // sostenibilidad: la deuda por encima del umbral castiga el ciclo (prima de riesgo / crowding out)
    const double exceso = std::max(0.0, deuda_pib_ - par_.umbral_deuda_pib);
    ciclo_ *= (1.0 - par_.penalidad_crecimiento_deuda * exceso);

    prev_pib_ = static_cast<double>(pib);
    productividad_ *= (1.0 + par_.productividad_anual);  // crecimiento real de ingresos
}

// --- métricas agregadas (solo agentes vivos) ---
MetricasAnuales Engine::medir(int anio) {
    const std::int64_t N = p_.size();
    MetricasAnuales m; m.anio = anio;
    std::int64_t vivos = 0, ocup = 0, desoc = 0, inf = 0, enfermos = 0, delinc = 0,
                 escolar = 0, asisten = 0, pobres = 0, pobres_ext = 0,
                 adultos = 0, con_deuda = 0, estresados = 0;
    long double suma_edad = 0, suma_sat = 0, suma_op = 0, suma_op2 = 0;
    std::vector<float> pcvec;   // ingreso per cápita del hogar (para Gini y pobreza, como DANE)
    for (std::int64_t i = 0; i < N; ++i) {
        if (!p_.vivo[i]) continue;
        ++vivos; suma_edad += p_.edad[i];
        suma_sat += p_.satisfaccion_vida[i];
        suma_op += p_.opinion_politica[i];
        suma_op2 += static_cast<long double>(p_.opinion_politica[i]) * p_.opinion_politica[i];
        if (p_.situacion_laboral[i] == SituacionLaboral::Ocupado)   { ++ocup; inf += p_.informal[i]; }
        if (p_.situacion_laboral[i] == SituacionLaboral::Desocupado) ++desoc;
        if (p_.meses_enfermo[i] > 0) ++enfermos;
        if (p_.es_delincuente[i]) ++delinc;
        if (p_.edad[i] >= 5 && p_.edad[i] <= 17) { ++escolar; if (p_.asiste_escuela[i]) ++asisten; }
        pcvec.push_back(hh_pc_[i]);
        if (hh_pc_[i] < static_cast<float>(par_.linea_pobreza_mensual)) ++pobres;
        if (hh_pc_[i] < static_cast<float>(par_.linea_pobreza_extrema)) ++pobres_ext;
        if (p_.edad[i] >= 18) {
            ++adultos;
            if (p_.deuda[i] > 0.1 * par_.linea_pobreza_mensual) ++con_deuda;
            if (p_.deuda[i] > par_.umbral_estres_financiero * std::max(hh_pc_[i], 1.0f) * 12.0f) ++estresados;
        }
    }
    m.poblacion = vivos;
    m.edad_media = vivos ? static_cast<Real>(suma_edad / vivos) : 0;
    m.desempleo = (ocup + desoc) ? static_cast<Real>(desoc) / (ocup + desoc) : 0;
    m.informalidad = ocup ? static_cast<Real>(inf) / ocup : 0;
    m.prev_enfermedad = vivos ? static_cast<Real>(enfermos) / vivos : 0;
    m.tasa_delincuencia = vivos ? static_cast<Real>(delinc) / vivos : 0;
    m.cobertura_educativa = escolar ? static_cast<Real>(asisten) / escolar : 0;
    m.tasa_desercion = escolar ? 1.0 - m.cobertura_educativa : 0;
    m.pobreza = vivos ? static_cast<Real>(pobres) / vivos : 0;
    m.pobreza_extrema = vivos ? static_cast<Real>(pobres_ext) / vivos : 0;
    // Gini del ingreso per cápita del hogar (definición DANE)
    std::sort(pcvec.begin(), pcvec.end());
    const std::int64_t n = static_cast<std::int64_t>(pcvec.size());
    long double w = 0, tot = 0;
    for (std::int64_t i = 0; i < n; ++i) { w += static_cast<long double>(i + 1) * pcvec[i]; tot += pcvec[i]; }
    m.gini_ingreso = tot > 0 ? static_cast<Real>((2.0L * w) / (n * tot) - static_cast<long double>(n + 1) / n) : 0;
    if (pib_base_ <= 0.0) pib_base_ = static_cast<double>(tot) > 0 ? static_cast<double>(tot) : 1.0;
    m.pib_index = static_cast<double>(tot) / pib_base_;
    m.crecimiento = crecimiento_;
    m.tasa_migracion = vivos ? static_cast<Real>(migraciones_anio_) / vivos : 0;
    m.recaudo_pib = recaudo_pib_; m.deficit_pib = deficit_pib_; m.deuda_pib = deuda_pib_;
    m.deuda_informal = adultos ? static_cast<Real>(con_deuda) / adultos : 0;
    m.estres_financiero = adultos ? static_cast<Real>(estresados) / adultos : 0;
    if (vivos) {
        m.satisfaccion_media = static_cast<Real>(suma_sat / vivos);
        const long double med = suma_op / vivos;
        m.polarizacion = static_cast<Real>(std::sqrt(std::max(0.0L, suma_op2 / vivos - med * med)));
    }
    return m;
}

static void escribir_fila(std::ostream& os, const MetricasAnuales& m) {
    os << m.anio << "," << m.poblacion << "," << m.edad_media << "," << m.desempleo << ","
       << m.informalidad << "," << m.gini_ingreso << "," << m.pobreza << "," << m.pobreza_extrema << ","
       << m.tasa_desercion << "," << m.prev_enfermedad << "," << m.tasa_delincuencia << ","
       << m.cobertura_educativa << "," << m.pib_index << "," << m.crecimiento << ","
       << m.tasa_migracion << "," << m.satisfaccion_media << "," << m.polarizacion << ","
       << m.recaudo_pib << "," << m.deficit_pib << "," << m.deuda_pib << ","
       << m.deuda_informal << "," << m.estres_financiero << "\n";
}

// Exporta métricas agregadas por departamento (estado actual del modelo).
void Engine::exportar_departamentos(std::ostream& os) {
    const std::int64_t N = p_.size();
    const std::int64_t D = g_.n_departamentos();
    std::vector<std::int64_t> pob(D,0), ocup(D,0), desoc(D,0), inf(D,0), pobres(D,0), delinc(D,0);
    for (std::int64_t i = 0; i < N; ++i) {
        if (!p_.vivo[i]) continue;
        const std::uint8_t d = p_.departamento_id[i];
        ++pob[d];
        if (p_.situacion_laboral[i] == SituacionLaboral::Ocupado)   { ++ocup[d]; inf[d] += p_.informal[i]; }
        if (p_.situacion_laboral[i] == SituacionLaboral::Desocupado) ++desoc[d];
        if (p_.es_delincuente[i]) ++delinc[d];
        if (hh_pc_[i] < static_cast<float>(par_.linea_pobreza_mensual)) ++pobres[d];
    }
    os << "cod_dpto,departamento,poblacion,desempleo,informalidad,pobreza,tasa_delincuencia\n";
    for (std::int64_t d = 0; d < D; ++d) {
        if (pob[d] == 0) continue;
        const double pea = ocup[d] + desoc[d];
        os << g_.dpto_codigo[d] << ",\"" << g_.dpto_nombre[d] << "\"," << pob[d] << ","
           << (pea ? desoc[d]/pea : 0.0) << ","
           << (ocup[d] ? static_cast<double>(inf[d])/ocup[d] : 0.0) << ","
           << static_cast<double>(pobres[d])/pob[d] << ","
           << static_cast<double>(delinc[d])/pob[d] << "\n";
    }
}

// Perfiles: agrega el estado actual por (departamento × sexo × grupo de edad × nivel
// educativo). Permite que una persona consulte "gente como yo, en promedio, así está".
static int edad_bin(int e) {
    if (e < 15) return 0; if (e < 25) return 1; if (e < 35) return 2; if (e < 45) return 3;
    if (e < 55) return 4; if (e < 65) return 5; return 6;
}
static const char* edad_label(int b) {
    static const char* L[7] = {"0-14","15-24","25-34","35-44","45-54","55-64","65+"}; return L[b];
}
static int educ_bin(NivelEducativo n) {
    int v = static_cast<int>(n);
    if (v <= static_cast<int>(NivelEducativo::Primaria)) return 0;          // bajo
    if (v <= static_cast<int>(NivelEducativo::Media)) return 1;             // medio
    return 2;                                                              // alto (técnico+)
}
static const char* educ_label(int b) {
    static const char* L[3] = {"bajo","medio","alto"}; return L[b];
}

void Engine::escribir_perfiles(int anio, bool header) {
    std::ostream& os = *perfiles_;
    if (header)
        os << "anio,cod_dpto,departamento,sexo,edad,educacion,n,ingreso_pc,"
              "p_ocupado,p_pobreza,p_enfermo,p_delincuencia,satisfaccion\n";
    const std::int64_t D = g_.n_departamentos();
    const int NB = static_cast<int>(D) * 2 * 7 * 3;
    auto idx = [](int d,int s,int e,int u){ return ((d*2+s)*7+e)*3+u; };
    std::vector<std::int64_t> n(NB,0), ocup(NB,0), pobre(NB,0), enf(NB,0), del(NB,0);
    std::vector<double> pc(NB,0.0), sat(NB,0.0);
    for (std::int64_t i = 0; i < p_.size(); ++i) {
        if (!p_.vivo[i]) continue;
        int c = idx(p_.departamento_id[i], static_cast<int>(p_.sexo[i]),
                    edad_bin(p_.edad[i]), educ_bin(static_cast<NivelEducativo>(p_.nivel_educativo[i])));
        ++n[c]; pc[c] += hh_pc_[i]; sat[c] += p_.satisfaccion_vida[i];
        ocup[c]  += (p_.situacion_laboral[i] == SituacionLaboral::Ocupado);
        pobre[c] += (hh_pc_[i] < static_cast<float>(par_.linea_pobreza_mensual));
        enf[c]   += (p_.meses_enfermo[i] > 0);
        del[c]   += p_.es_delincuente[i];
    }
    for (int d = 0; d < D; ++d)
      for (int s = 0; s < 2; ++s)
        for (int e = 0; e < 7; ++e)
          for (int u = 0; u < 3; ++u) {
            int c = idx(d,s,e,u);
            if (n[c] < 20) continue;   // omite celdas con muestra muy chica
            os << anio << "," << g_.dpto_codigo[d] << ",\"" << g_.dpto_nombre[d] << "\","
               << (s==0?"Hombre":"Mujer") << "," << edad_label(e) << "," << educ_label(u) << ","
               << n[c] << "," << static_cast<long long>(pc[c]/n[c]) << ","
               << static_cast<double>(ocup[c])/n[c] << "," << static_cast<double>(pobre[c])/n[c] << ","
               << static_cast<double>(enf[c])/n[c] << "," << static_cast<double>(del[c])/n[c] << ","
               << sat[c]/n[c] << "\n";
          }
}

void Engine::run(std::ostream& csv) {
    csv << "anio,poblacion,edad_media,desempleo,informalidad,gini_ingreso,pobreza,pobreza_extrema,"
           "tasa_desercion,prev_enfermedad,tasa_delincuencia,cobertura_educativa,"
           "pib_index,crecimiento,tasa_migracion,satisfaccion_media,polarizacion,"
           "recaudo_pib,deficit_pib,deuda_pib,deuda_informal,estres_financiero\n";

    // estado inicial (año base): fijar empleo/ingreso primero
    mercado_laboral();
    recomputar_ingreso_hogar();
    prev_pib_ = 0.0; cerrar_macro();   // fija PIB base
    escribir_fila(csv, medir(cfg_.anio_inicial));
    if (perfiles_) escribir_perfiles(cfg_.anio_inicial, true);

    for (int a = 1; a <= cfg_.horizonte_anios; ++a) {
        // shock exógeno (p.ej. COVID-2020): colapso transitorio del empleo ese año
        shock_actual_ = (cfg_.anio_inicial + a == cfg_.shock_anio) ? (1.0 - cfg_.shock_mag) : 1.0;
        demografia();            // M7
        educacion();             // M1
        dinamica_empresas();     // §3: extorsión/quiebra/entrada (fija la capacidad de empleo)
        mercado_laboral();       // M2/M3 (empleo endógeno a las empresas)
        recomputar_ingreso_hogar();
        delincuencia();          // M5
        migracion();             // M-migración (§6.3)
        recomputar_ingreso_hogar();   // hogares cambiaron por migración
        finanzas();              // crédito de hogares (gota a gota / formal)
        opinion();               // M-opinión/bienestar (§6.6)
        for (int paso = 0; paso < cfg_.pasos_por_anio; ++paso) {
            salud_mensual();     // M4
            economia_mensual();  // KWEM
        }
        cerrar_macro();          // §8 paso 12: agrega y realimenta el ciclo
        escribir_fila(csv, medir(cfg_.anio_inicial + a));
        if (perfiles_) escribir_perfiles(cfg_.anio_inicial + a, false);
    }
}

} // namespace socium
