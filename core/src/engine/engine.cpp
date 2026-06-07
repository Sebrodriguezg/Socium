#include "socium/engine.hpp"
#include "socium/society.hpp"
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
    }
    return pol;
}

Engine::Engine(Population& p, Households& h, const Geography& g, Parametros par, EngineConfig cfg,
               Politicas pol)
    : p_(p), h_(h), g_(g), par_(par), cfg_(cfg), pol_(pol) {
    #pragma omp parallel
    { seed_thread_rng(cfg_.seed); }
    // política de aseguramiento: afiliar a una fracción de los sin afiliación (M4)
    if (pol_.aseguramiento_boost > 0.0) {
        const std::int64_t N = p_.size();
        for (std::int64_t i = 0; i < N; ++i)
            if (p_.afiliacion_salud[i] == AfiliacionSalud::Ninguno && uniform01() < pol_.aseguramiento_boost)
                p_.afiliacion_salud[i] = AfiliacionSalud::Subsidiado;
    }
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
        suma[p_.hogar_id[i]] += p_.ingreso_laboral[i];
        cnt[p_.hogar_id[i]]++;
    }
    hh_pc_.assign(N, 0.0f);
    for (std::int64_t i = 0; i < N; ++i) {
        const std::int32_t hid = p_.hogar_id[i];
        float pc = cnt[hid] > 0 ? static_cast<float>(suma[hid] / cnt[hid]) : 0.0f;
        hh_pc_[i] = pc + static_cast<float>(pol_.transfer_ingreso_pc);  // transferencias (política)
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

// M2/M3 — empleo, informalidad e ingreso (Mincer)
void Engine::mercado_laboral() {
    const std::int64_t N = p_.size();
    #pragma omp parallel for schedule(static)
    for (std::int64_t i = 0; i < N; ++i) {
        if (!p_.vivo[i]) continue;
        const int edad = p_.edad[i];
        if (edad < 15) { p_.situacion_laboral[i] = SituacionLaboral::MenorEdad; p_.ingreso_laboral[i] = 0; continue; }
        if (p_.asiste_escuela[i]) { p_.situacion_laboral[i] = SituacionLaboral::Inactivo; p_.ingreso_laboral[i] = 0; continue; }

        const int anios = p_.anios_escolaridad[i];
        double p_part = (0.55 + 0.03 * static_cast<int>(p_.nivel_educativo[i])) * pol_.empleo_mult;
        if (edad > 65) p_part *= 0.4;
        if (uniform01() > p_part) { p_.situacion_laboral[i] = SituacionLaboral::Inactivo; p_.ingreso_laboral[i] = 0; continue; }

        // entre activos: desempleo objetivo (menor a mayor educación)
        double pdesemp = par_.desempleo_objetivo * (anios >= 11 ? 0.8 : 1.4);
        if (uniform01() < pdesemp) { p_.situacion_laboral[i] = SituacionLaboral::Desocupado; p_.ingreso_laboral[i] = 0; continue; }

        // ocupado: ingreso Mincer + residual lognormal (dispersión salarial real)
        p_.situacion_laboral[i] = SituacionLaboral::Ocupado;
        const int exper = std::max(0, edad - anios - 6);
        std::normal_distribution<double> ruido(0.0, 0.55);   // residual de Mincer
        double ln = par_.retorno_anual_escolaridad * (anios - 11) + 0.03 * exper
                  - 0.0004 * exper * exper + ruido(thread_rng());
        double ingreso = par_.smlv * pol_.smlv_mult * std::exp(ln);
        // informalidad (M3): mayor a menor educación (calibrada a ~57% nacional)
        p_.informal[i] = (uniform01() < (anios < 11 ? 0.66 : 0.30)) ? 1 : 0;
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

void Engine::economia_mensual() {
    EconomyParams ep;
    ep.rule = cfg_.regla_economia;
    ep.lambda = cfg_.lambda;
    simular_economia(p_, ep, 2, cfg_.economia_local, cfg_.seed);
}

// --- métricas agregadas (solo agentes vivos) ---
MetricasAnuales Engine::medir(int anio) {
    const std::int64_t N = p_.size();
    MetricasAnuales m; m.anio = anio;
    std::int64_t vivos = 0, ocup = 0, desoc = 0, inf = 0, enfermos = 0, delinc = 0,
                 escolar = 0, asisten = 0, pobres = 0;
    long double suma_edad = 0;
    std::vector<float> ingresos;
    for (std::int64_t i = 0; i < N; ++i) {
        if (!p_.vivo[i]) continue;
        ++vivos; suma_edad += p_.edad[i];
        if (p_.situacion_laboral[i] == SituacionLaboral::Ocupado)   { ++ocup; ingresos.push_back(p_.ingreso_laboral[i]); inf += p_.informal[i]; }
        if (p_.situacion_laboral[i] == SituacionLaboral::Desocupado) ++desoc;
        if (p_.meses_enfermo[i] > 0) ++enfermos;
        if (p_.es_delincuente[i]) ++delinc;
        if (p_.edad[i] >= 5 && p_.edad[i] <= 17) { ++escolar; if (p_.asiste_escuela[i]) ++asisten; }
        if (hh_pc_[i] < static_cast<float>(par_.linea_pobreza_mensual)) ++pobres;
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
    // gini ingreso
    std::sort(ingresos.begin(), ingresos.end());
    const std::int64_t n = static_cast<std::int64_t>(ingresos.size());
    long double w = 0, tot = 0;
    for (std::int64_t i = 0; i < n; ++i) { w += static_cast<long double>(i + 1) * ingresos[i]; tot += ingresos[i]; }
    m.gini_ingreso = tot > 0 ? static_cast<Real>((2.0L * w) / (n * tot) - static_cast<long double>(n + 1) / n) : 0;
    return m;
}

static void escribir_fila(std::ostream& os, const MetricasAnuales& m) {
    os << m.anio << "," << m.poblacion << "," << m.edad_media << "," << m.desempleo << ","
       << m.informalidad << "," << m.gini_ingreso << "," << m.pobreza << ","
       << m.tasa_desercion << "," << m.prev_enfermedad << "," << m.tasa_delincuencia << ","
       << m.cobertura_educativa << "\n";
}

void Engine::run(std::ostream& csv) {
    csv << "anio,poblacion,edad_media,desempleo,informalidad,gini_ingreso,pobreza,"
           "tasa_desercion,prev_enfermedad,tasa_delincuencia,cobertura_educativa\n";

    // estado inicial (año base): fijar empleo/ingreso primero
    mercado_laboral();
    recomputar_ingreso_hogar();
    escribir_fila(csv, medir(cfg_.anio_inicial));

    for (int a = 1; a <= cfg_.horizonte_anios; ++a) {
        demografia();            // M7
        educacion();             // M1
        mercado_laboral();       // M2/M3
        recomputar_ingreso_hogar();
        delincuencia();          // M5
        for (int paso = 0; paso < cfg_.pasos_por_anio; ++paso) {
            salud_mensual();     // M4
            economia_mensual();  // KWEM
        }
        escribir_fila(csv, medir(cfg_.anio_inicial + a));
    }
}

} // namespace socium
