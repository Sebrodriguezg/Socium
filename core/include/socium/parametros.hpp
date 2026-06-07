#pragma once
// Parámetros de los mecanismos causales (M1-M7). Valores por defecto = los citados en
// data/reference/parametros.yaml (fuentes públicas). Lo marcado "supuesto" se calibra
// contra un agregado real. Ver docs/model-spec/mecanismos.md
#include "socium/economy.hpp"  // Real

namespace socium {

struct Parametros {
    // --- M1 deserción educativa ---
    Real desercion_estrato1 = 0.133;   // LEE Javeriana 2021-2 (superior)
    Real desercion_estrato6 = 0.071;   // LEE Javeriana 2021-2
    Real desercion_base_5_16 = 0.037;  // MEN nacional

    // --- M2 retornos a la educación (Mincer) ---
    Real retorno_anual_escolaridad = 0.10;  // GEIH (~9.5%-11.3%)
    Real penalizacion_informalidad = 0.30;  // supuesto, calibrar GEIH
    Real brecha_genero = 0.12;               // supuesto, calibrar GEIH
    Real smlv = 1'423'500.0;                 // salario mínimo 2025 (COP/mes), referencia
    Real sigma_ingreso = 0.95;               // dispersión lognormal — calibrado a Gini DANE 0.553
    Real calib_ingreso = 1.8;                // escala de recursos del hogar (perceptores por hogar
                                             // sub-contados + otros ingresos) — calibrado a pobreza GEIH

    // --- M3 mercado laboral ---
    Real desempleo_objetivo = 0.10;    // GEIH
    Real informalidad_objetivo = 0.57; // GEIH

    // --- M4 salud ---
    Real cobertura_aseguramiento = 0.97;       // MinSalud
    Real rr_enfermar_sin_aseguramiento = 1.5;  // supuesto
    Real prob_base_enfermar_mensual = 0.02;    // supuesto, calibrar
    Real perdida_productividad_enfermo = 0.4;  // supuesto

    // --- M5 delincuencia (riesgos relativos = exp(β) de spec §6.4; base se normaliza
    //     a la tasa real de homicidios/hurtos municipales) ---
    Real crimen_prob_base = 0.01;      // ancla a homicidios/hurtos municipales (Med.Legal)
    Real rr_nini = 2.34;               // exp(0.85) — desempleo largo / nini
    Real rr_pobreza = 1.86;            // exp(0.62) — estrato bajo
    Real rr_baja_educacion = 1.79;     // exp(0.58) — deserción <9°
    Real rr_hombre_joven = 3.0;        // estructura etaria-sexo del delito
    Real rr_presencia_gao = 3.06;      // exp(1.12) — presencia GAO (requiere dataset; aún no aplicado)
    Real prob_abandono_delito = 0.10;  // salir del delito por año (con oportunidades)

    // --- M6 movilidad ---
    Real persistencia_intergeneracional = 0.6; // supuesto, ELCA

    // --- M7 demografía ---
    Real fecundidad_total = 1.75;      // DANE
    Real esperanza_vida = 77.0;        // DANE

    // --- ingresos no laborales del hogar (para pobreza realista, M3/§2.4) ---
    Real pension_contributiva = 1'100'000.0; // jubilados con historia formal (COP/mes)
    Real colombia_mayor = 80'000.0;          // subsidio a adultos mayores vulnerables
    Real subsistencia_informal = 380'000.0;  // rebusque de no ocupados en edad laboral
    Real ingreso_no_laboral_pc = 140'000.0;  // piso per cápita del hogar (transferencias/remesas/
                                             // rentas/en especie) — calibrado a pobreza GEIH/DANE

    // --- cierre micro-macro (§8 paso 12, §11 Brecha 2) ---
    Real productividad_anual = 0.015;        // crecimiento real de productividad (PIB/ocupado)
    Real sensibilidad_ciclo = 0.5;           // cuánto realimenta el crecimiento al empleo

    // --- M-migración interna por gravedad (spec §6.3) ---
    Real migracion_base = 0.012;             // prob anual base de migrar (económica voluntaria)
    Real push_conflicto = 4.0;               // el conflicto del origen empuja (desplazamiento forzado)
    Real gravity_alpha = 0.85;               // exponente de masa poblacional del destino

    // --- M-opinión / bienestar / radicalización (spec §6.6) ---
    Real conformidad_social = 0.30;          // J: fuerza de conformidad con el municipio
    Real peso_seguridad_bienestar = 3.0;     // cuánto pesa el conflicto local en la satisfacción

    // --- agente EMPRESA (spec §3) ---
    Real empleos_objetivo_frac = 0.52;       // puestos ≈ frac · pob. en edad de trabajar — calibra desempleo a ~10% (GEIH)
    Real margen_trabajador_anual = 8'000'000.0; // valor agregado bruto por trabajador (COP/año)
    Real margen_operativo = 0.15;            // utilidad operativa sobre valor agregado
    Real extorsion_tasa = 0.20;              // 'vacuna' como fracción del bruto en zona de conflicto
    Real prob_entrada_empresa = 0.12;        // reactivación/emprendimiento en buen ciclo

    // --- líneas de referencia (DANE 2023) ---
    Real linea_pobreza_mensual = 435'375.0;  // línea de pobreza monetaria per cápita
    Real linea_pobreza_extrema = 199'828.0;  // línea de pobreza extrema per cápita
};

} // namespace socium
