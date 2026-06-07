#!/usr/bin/env python3
"""
Traduce un escenario (experiments/scenarios/<id>.yaml) a los parámetros numéricos que
consume el motor (data/reference/escenario_<id>.flat, formato `clave valor`).

El mapeo propuesta->parámetro es de PRIMER ORDEN y documentado en
docs/escenarios/traduccion.md. Es transparente y ajustable: cada coeficiente abajo
representa un supuesto de modelado, no un hecho del programa.

    python3 scripts/build_scenario_params.py cepeda
    python3 scripts/build_scenario_params.py de_la_espriella
    python3 scripts/build_scenario_params.py baseline
"""
from __future__ import annotations
import sys, yaml
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
SCN = ROOT / "experiments" / "scenarios"


def lever(pol, dom, key, field="valor", default=None):
    try:
        return pol[dom][key][field]
    except (KeyError, TypeError):
        return default


def mapear(pol: dict) -> dict:
    """Mapeo de palancas de política -> knobs del motor (primer orden, documentado)."""
    k = {"transfer_ingreso_pc": 0.0, "desercion_mult": 1.0, "salud_prob_mult": 1.0,
         "aseguramiento_boost": 0.0, "crimen_base_mult": 1.0, "crimen_abandono_mult": 1.0,
         "smlv_mult": 1.0, "empleo_mult": 1.0}

    # --- transferencias -> ingreso per cápita del hogar ---
    monto = lever(pol, "transferencias", "monto_per_capita", default=1.0) or 1.0
    cob   = lever(pol, "transferencias", "cobertura", default=1.0) or 1.0
    k["transfer_ingreso_pc"] = 80000.0 * (monto * cob - 1.0)
    if lever(pol, "transferencias", "banco_del_pueblo", default=False):
        k["transfer_ingreso_pc"] += 20000.0
    if lever(pol, "transferencias", "madres_cabeza_hogar", default=None):
        k["transfer_ingreso_pc"] += 8000.0   # focalizado: efecto pc nacional pequeño

    # --- educación -> deserción ---
    inv_edu = lever(pol, "educacion", "inversion_pib", default=1.0) or 1.0
    k["desercion_mult"] = 1.0 / inv_edu
    if lever(pol, "educacion", "gratuidad_superior", default=False) is True:
        k["desercion_mult"] *= 0.85

    # --- salud -> probabilidad de enfermar + aseguramiento ---
    inv_sal = lever(pol, "salud", "inversion_pib", default=1.0) or 1.0
    k["salud_prob_mult"] = 1.0 / inv_sal
    modelo = lever(pol, "salud", "modelo", default="continuidad")
    if isinstance(modelo, str) and "liquidacion" in modelo:
        k["salud_prob_mult"] *= 1.05            # disrupción transitoria del aseguramiento
    if inv_sal > 1.0:
        k["aseguramiento_boost"] = 0.3

    # --- seguridad -> crimen ---
    enf = lever(pol, "seguridad", "enfoque", default="continuidad")
    if enf == "mano_dura":
        k["crimen_base_mult"] = 0.75; k["crimen_abandono_mult"] = 0.9
    elif isinstance(enf, str) and "seguridad_humana" in enf:
        k["crimen_base_mult"] = 0.95; k["crimen_abandono_mult"] = 1.6

    # --- laboral ---
    k["smlv_mult"] = lever(pol, "laboral", "salario_minimo_real", default=1.0) or 1.0
    k["empleo_mult"] = lever(pol, "laboral", "formalizacion", default=1.0) or 1.0

    return k


def main():
    if len(sys.argv) < 2:
        sys.exit("uso: build_scenario_params.py <id-escenario>")
    sid = sys.argv[1]
    doc = yaml.safe_load((SCN / f"{sid}.yaml").read_text(encoding="utf-8"))
    pol = doc.get("politicas") or {}
    knobs = mapear(pol)

    out = ROOT / "data" / "reference" / f"escenario_{sid}.flat"
    with out.open("w", encoding="utf-8") as f:
        f.write(f"# escenario {sid} -> knobs del motor (generado; ver docs/escenarios/traduccion.md)\n")
        for key, val in knobs.items():
            f.write(f"{key} {val:.6g}\n")
    print(f"OK -> {out.relative_to(ROOT)}")
    for key, val in knobs.items():
        print(f"   {key:22} {val:.4g}")


if __name__ == "__main__":
    main()
