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
#include "socium/firm.hpp"
#include "socium/geography.hpp"
#include "socium/parametros.hpp"
#include "socium/economy.hpp"

namespace socium { struct Network; }

namespace socium {

struct EngineConfig {
    int anio_inicial    = 2026;
    int horizonte_anios = 4;          // 2026 -> 2030
    int pasos_por_anio  = 12;         // dt: 1=anual, 12=mensual, 52=semanal, 365=diario
    std::uint64_t seed  = 42;
    Rule regla_economia = Rule::SavingPropensity;
    Real lambda         = 0.5;
    bool economia_local = true;       // KWEM por municipio
    int  shock_anio     = 0;          // año de shock exógeno (0 = ninguno; p.ej. 2020 COVID)
    Real shock_mag      = 0.0;        // magnitud del shock al empleo ese año (0-1)
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
    double impuesto_mult         = 1.0;  // tasa tributaria (fiscal)
    double gasto_mult            = 1.0;  // gasto público base (fiscal)
    double boost_agro            = 1.0;  // impulso de demanda al sector agro (insumo-producto)
    double boost_mineria         = 1.0;  // impulso de demanda al sector minero/extractivo

    static Politicas load(const std::string& path);  // archivo key=value
};

struct MetricasAnuales {
    int anio = 0;
    std::int64_t poblacion = 0;
    Real edad_media = 0, desempleo = 0, informalidad = 0, gini_ingreso = 0,
         pobreza = 0, pobreza_extrema = 0, tasa_desercion = 0, prev_enfermedad = 0,
         tasa_delincuencia = 0, cobertura_educativa = 0,
         pib_index = 0, crecimiento = 0, tasa_migracion = 0,
         satisfaccion_media = 0, polarizacion = 0,
         recaudo_pib = 0, deficit_pib = 0, deuda_pib = 0,
         deuda_informal = 0, estres_financiero = 0;
};

class Engine {
public:
    Engine(Population& p, Households& h, Firms& f, const Geography& g, Parametros par,
           EngineConfig cfg, Politicas pol = {});

    void run(std::ostream& csv);          // simula el horizonte y escribe la serie anual
    MetricasAnuales medir(int anio);
    void exportar_departamentos(std::ostream& os);  // métricas por departamento (estado actual)
    void exportar_distribucion(std::ostream& os);   // concentración de ingreso + tamaño de empresas (stylized facts)
    // Activa la exportación de PERFILES (depto×sexo×edad×educación) por año -> consulta pública.
    void set_salida_perfiles(std::ostream* os) { perfiles_ = os; }
    // Activa la red social explícita (efecto de pares en crimen y opinión).
    void set_red(const Network* r) { red_ = r; }

private:
    // mecanismos
    void demografia();          // M7 anual
    void educacion();           // M1 anual
    void mercado_laboral();     // M2/M3 anual
    void salud_mensual();       // M4 mensual
    void delincuencia();        // M5 anual
    void migracion();           // M-migración: gravedad, huye del conflicto (§6.3)
    void finanzas();            // crédito de hogares: gota a gota / formal, estrés financiero
    void actualizar_produccion(); // insumo-producto (Leontief): propaga impulsos sectoriales
    void opinion();             // M-opinión/bienestar/radicalización (§6.6)
    void dinamica_empresas();   // §3: extorsión, quiebra y entrada de empresas
    void economia_mensual();    // economía (KWEM) mensual
    void cerrar_macro();        // §8 paso 12: agrega PIB/empleo y realimenta el ciclo
    void recomputar_ingreso_hogar();
    void escribir_perfiles(int anio, bool header);  // agrega por perfil para consulta pública

    Population&      p_;
    Households&      h_;
    Firms&           f_;
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
    double shock_actual_ = 1.0;  // factor de shock exógeno del año en curso (1 = sin shock)
    double deuda_ = 0.0;         // stock de deuda pública (unidades de PIB-proxy)
    double recaudo_pib_ = 0.0, deficit_pib_ = 0.0, deuda_pib_ = 0.0;  // últimos ratios fiscales
    std::ostream* perfiles_ = nullptr;  // salida opcional de perfiles por año
    const Network* red_ = nullptr;      // red social explícita (opcional)
    double sector_output_[7] = {1,1,1,1,1,1,1};  // índice de producción por sector (insumo-producto)
    double xbase_[7] = {0,0,0,0,0,0,0};          // producción base por sector (referencia)
    std::int32_t next_hogar_id_ = 0;  // siguiente id de hogar (para migrantes)
    std::int64_t migraciones_anio_ = 0;
};

} // namespace socium
