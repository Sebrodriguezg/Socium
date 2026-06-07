// Simulación temporal de Colombia 2026 -> 2026+horizonte.
//   socium_simular [--n N] [--years Y] [--pasos P] [--rule ...] [--local 0|1]
//                  [--seed S] [--threads T] [--out archivo.csv]
// Construye la población base, forma hogares y corre el motor (M1-M7). Escribe la
// serie anual de métricas a CSV (o stdout).
#include "socium/population.hpp"
#include "socium/geography.hpp"
#include "socium/household.hpp"
#include "socium/firm.hpp"
#include "socium/engine.hpp"

#include <chrono>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#ifdef _OPENMP
#include <omp.h>
#endif

using namespace socium;

static const char* arg(int argc, char** argv, const char* k, const char* d) {
    for (int i = 1; i + 1 < argc; ++i)
        if (std::strcmp(argv[i], k) == 0) return argv[i + 1];
    return d;
}

int main(int argc, char** argv) {
    EngineConfig cfg;
    const std::int64_t n = std::atoll(arg(argc, argv, "--n", "5000000"));
    cfg.horizonte_anios = std::atoi(arg(argc, argv, "--years", "4"));
    cfg.pasos_por_anio  = std::atoi(arg(argc, argv, "--pasos", "12"));
    cfg.seed            = std::strtoull(arg(argc, argv, "--seed", "42"), nullptr, 10);
    cfg.regla_economia  = rule_from_string(arg(argc, argv, "--rule", "saving"));
    cfg.lambda          = std::atof(arg(argc, argv, "--lambda", "0.5"));
    cfg.economia_local  = std::atoi(arg(argc, argv, "--local", "1")) != 0;
#ifdef _OPENMP
    const char* th = arg(argc, argv, "--threads", nullptr);
    if (th) omp_set_num_threads(std::atoi(th));
#endif

    std::cerr << "Construyendo Colombia base (" << n << " agentes)...\n";
    Geography g = Geography::load_csv(arg(argc, argv, "--geo", "data/reference/divipola.csv"));
    g.cargar_pesos(arg(argc, argv, "--pob", "data/reference/poblacion_municipal.csv"));
    g.cargar_conflicto(arg(argc, argv, "--conflicto", "data/reference/conflicto_municipal.csv"));
    Population p = build_synthetic(n, cfg.seed);
    asignar_municipios(p, g, cfg.seed);
    ajustar_educacion_espacial(p, g);   // desigualdad educativa rural/urbana (spec §2.3)
    Households h = form_households(p, g, cfg.seed);

    Parametros par;  // valores con fuente (data/reference/parametros.yaml)
    // overrides de calibración (si se pasan; si no, usan los defaults calibrados de Parametros)
    if (const char* v = arg(argc, argv, "--calib-ingreso", nullptr)) par.calib_ingreso = std::atof(v);
    if (const char* v = arg(argc, argv, "--sigma", nullptr))         par.sigma_ingreso = std::atof(v);
    if (const char* v = arg(argc, argv, "--base-pc", nullptr))       par.ingreso_no_laboral_pc = std::atof(v);
    if (const char* v = arg(argc, argv, "--empleos-frac", nullptr))  par.empleos_objetivo_frac = std::atof(v);
    // empresas: puestos ≈ frac · población en edad de trabajar (15-65)
    std::int64_t pet = 0;
    for (std::int64_t i = 0; i < p.size(); ++i) if (p.edad[i] >= 15 && p.edad[i] <= 65) ++pet;
    Firms f = crear_empresas(g, static_cast<std::int64_t>(par.empleos_objetivo_frac * pet), cfg.seed);
    std::cerr << "empresas: " << f.size() << " (" << metrics::empleos_ofrecidos(f) << " empleos)\n";
    Politicas pol;
    const char* esc = arg(argc, argv, "--escenario", nullptr);
    if (esc) { pol = Politicas::load(esc); std::cerr << "escenario: " << esc << "\n"; }
    Engine eng(p, h, f, g, par, cfg, pol);

    const char* out = arg(argc, argv, "--out", nullptr);
    std::ofstream fout;
    if (out) fout.open(out);
    std::ostream& os = out ? fout : std::cout;

    std::cerr << "Simulando " << cfg.horizonte_anios << " años (dt = "
              << cfg.pasos_por_anio << "/año)...\n";
    const auto t0 = std::chrono::steady_clock::now();
    eng.run(os);
    const auto t1 = std::chrono::steady_clock::now();
    std::cerr << "Listo en " << std::chrono::duration<double>(t1 - t0).count() << " s\n";
    if (out) std::cerr << "serie -> " << out << "\n";

    if (const char* od = arg(argc, argv, "--out-deptos", nullptr)) {
        std::ofstream fd(od); eng.exportar_departamentos(fd);
        std::cerr << "departamentos -> " << od << "\n";
    }
    return 0;
}
