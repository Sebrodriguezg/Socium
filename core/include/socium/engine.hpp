#pragma once
// Motor de tiempo multi-escala: simula la Colombia 2026 hacia adelante ejecutando los
// mecanismos causales M1-M7 (ver docs/model-spec/mecanismos.md) a su frecuencia natural.
// dt configurable: lo lento (educación, demografía) corre anual; lo rápido (salud,
// economía) corre `pasos_por_anio` veces por año (mensual por defecto).
#include <cstdint>
#include <ostream>
#include <vector>

#include "socium/population.hpp"
#include "socium/household.hpp"
#include "socium/geography.hpp"
#include "socium/parametros.hpp"
#include "socium/economy.hpp"

namespace socium {

struct EngineConfig {
    int anio_inicial    = 2026;
    int horizonte_anios = 4;          // 2026 -> 2030
    int pasos_por_anio  = 12;         // dt: 1=anual, 12=mensual, 52=semanal, 365=diario
    std::uint64_t seed  = 42;
    Rule regla_economia = Rule::SavingPropensity;
    Real lambda         = 0.5;
    bool economia_local = true;       // KWEM por municipio
};

// Palancas de política que modifican los mecanismos (neutral = baseline).
// Se generan desde experiments/scenarios/<id>.yaml vía scripts/build_scenario_params.py
// (ver docs/escenarios/traduccion.md para el mapeo propuesta->parámetro).
struct Politicas {
    double transfer_ingreso_pc   = 0.0;  // COP/mes añadidos al ingreso per cápita del hogar
    double desercion_mult        = 1.0;  // <1 => menos deserción (M1)
    double salud_prob_mult       = 1.0;  // <1 => menos enfermedad (M4)
    double aseguramiento_boost   = 0.0;  // prob. de asegurar a los sin afiliación (M4)
    double crimen_base_mult      = 1.0;  // <1 => menos crimen base (M5)
    double crimen_abandono_mult  = 1.0;  // >1 => más reinserción (M5)
    double smlv_mult             = 1.0;  // salario mínimo real (M2)
    double empleo_mult           = 1.0;  // participación/empleo (M3)

    static Politicas load(const std::string& path);  // archivo key=value
};

struct MetricasAnuales {
    int anio = 0;
    std::int64_t poblacion = 0;
    Real edad_media = 0, desempleo = 0, informalidad = 0, gini_ingreso = 0,
         pobreza = 0, tasa_desercion = 0, prev_enfermedad = 0,
         tasa_delincuencia = 0, cobertura_educativa = 0,
         pib_index = 0, crecimiento = 0, tasa_migracion = 0;
};

class Engine {
public:
    Engine(Population& p, Households& h, const Geography& g, Parametros par, EngineConfig cfg,
           Politicas pol = {});

    void run(std::ostream& csv);          // simula el horizonte y escribe la serie anual
    MetricasAnuales medir(int anio);

private:
    // mecanismos
    void demografia();          // M7 anual
    void educacion();           // M1 anual
    void mercado_laboral();     // M2/M3 anual
    void salud_mensual();       // M4 mensual
    void delincuencia();        // M5 anual
    void migracion();           // M-migración: gravedad, huye del conflicto (§6.3)
    void economia_mensual();    // economía (KWEM) mensual
    void cerrar_macro();        // §8 paso 12: agrega PIB/empleo y realimenta el ciclo
    void recomputar_ingreso_hogar();

    Population&      p_;
    Households&      h_;
    const Geography& g_;
    Parametros       par_;
    EngineConfig     cfg_;
    Politicas        pol_;
    std::vector<float> hh_pc_;   // ingreso per cápita del hogar de cada persona
    // estado macro (cierre micro-macro)
    double prev_pib_ = 0.0;      // PIB agregado del año anterior
    double ciclo_ = 1.0;         // multiplicador de ciclo económico (realimenta empleo)
    double productividad_ = 1.0; // factor acumulado de productividad (crece ingresos)
    double crecimiento_ = 0.0;   // último crecimiento del PIB
    double pib_base_ = 0.0;      // PIB del año base (para índice)
    std::int32_t next_hogar_id_ = 0;  // siguiente id de hogar (para migrantes)
    std::int64_t migraciones_anio_ = 0;
};

} // namespace socium
