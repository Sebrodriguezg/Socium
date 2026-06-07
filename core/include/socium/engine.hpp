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

struct MetricasAnuales {
    int anio = 0;
    std::int64_t poblacion = 0;
    Real edad_media = 0, desempleo = 0, informalidad = 0, gini_ingreso = 0,
         pobreza = 0, tasa_desercion = 0, prev_enfermedad = 0,
         tasa_delincuencia = 0, cobertura_educativa = 0;
};

class Engine {
public:
    Engine(Population& p, Households& h, const Geography& g, Parametros par, EngineConfig cfg);

    void run(std::ostream& csv);          // simula el horizonte y escribe la serie anual
    MetricasAnuales medir(int anio);

private:
    // mecanismos
    void demografia();          // M7 anual
    void educacion();           // M1 anual
    void mercado_laboral();     // M2/M3 anual
    void salud_mensual();       // M4 mensual
    void delincuencia();        // M5 anual
    void economia_mensual();    // economía (KWEM) mensual
    void recomputar_ingreso_hogar();

    Population&      p_;
    Households&      h_;
    const Geography& g_;
    Parametros       par_;
    EngineConfig     cfg_;
    std::vector<float> hh_pc_;   // ingreso per cápita del hogar de cada persona
};

} // namespace socium
