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

    // --- M3 mercado laboral ---
    Real desempleo_objetivo = 0.10;    // GEIH
    Real informalidad_objetivo = 0.57; // GEIH

    // --- M4 salud ---
    Real cobertura_aseguramiento = 0.97;       // MinSalud
    Real rr_enfermar_sin_aseguramiento = 1.5;  // supuesto
    Real prob_base_enfermar_mensual = 0.02;    // supuesto, calibrar
    Real perdida_productividad_enfermo = 0.4;  // supuesto

    // --- M5 delincuencia (riesgos relativos; base se normaliza a la tasa real) ---
    Real crimen_prob_base = 0.01;      // supuesto, ancla a homicidios/hurtos municipales
    Real rr_nini = 2.5;
    Real rr_pobreza = 2.0;
    Real rr_baja_educacion = 1.8;
    Real rr_hombre_joven = 3.0;
    Real prob_abandono_delito = 0.10;  // supuesto: salir del delito por año (con oportunidades)

    // --- M6 movilidad ---
    Real persistencia_intergeneracional = 0.6; // supuesto, ELCA

    // --- M7 demografía ---
    Real fecundidad_total = 1.75;      // DANE
    Real esperanza_vida = 77.0;        // DANE

    // --- líneas de referencia ---
    Real linea_pobreza_mensual = 450'000.0;  // línea de pobreza monetaria per cápita (aprox 2024)
};

} // namespace socium
