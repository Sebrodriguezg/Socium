#!/usr/bin/env python3
"""
Compara las series de los escenarios (experiments/runs/serie_<id>.csv) y genera:
  - results/comparacion_escenarios.md  (tabla de trayectorias + deltas vs baseline)
  - results/fig_<metrica>.png           (si matplotlib está disponible)

    python3 python/socium/viz/comparar_escenarios.py
"""
from __future__ import annotations
import csv
from pathlib import Path

ROOT = Path(__file__).resolve().parents[3]
RUNS = ROOT / "experiments" / "runs"
RES = ROOT / "results"
ESCENARIOS = ["baseline", "cepeda", "de_la_espriella"]
METRICAS = ["pobreza", "gini_ingreso", "desempleo", "informalidad",
            "tasa_delincuencia", "cobertura_educativa", "prev_enfermedad",
            "deficit_pib", "deuda_pib"]


def cargar():
    data = {}
    for s in ESCENARIOS:
        f = RUNS / f"serie_{s}.csv"
        if f.exists():
            data[s] = list(csv.DictReader(open(f)))
    return data


def main():
    data = cargar()
    if "baseline" not in data:
        raise SystemExit("Faltan series. Corre socium_simular --escenario ... primero.")
    RES.mkdir(exist_ok=True)
    anios = [r["anio"] for r in data["baseline"]]

    out = ["# Comparación de escenarios — Colombia 2026→2030\n",
           "_Generado por comparar_escenarios.py. Diferencias trazables a las políticas; "
           "niveles absolutos por calibrar._\n"]

    # tabla final-año + delta vs baseline
    out.append(f"## Año {anios[-1]} (final) — nivel y Δ vs baseline\n")
    out.append("| Métrica | baseline | cepeda | Δ | de_la_espriella | Δ |")
    out.append("|---|---|---|---|---|---|")
    fin = {s: data[s][-1] for s in data}
    for m in METRICAS:
        b = float(fin["baseline"][m])
        row = f"| {m} | {b:.4f} |"
        for s in ["cepeda", "de_la_espriella"]:
            if s in fin:
                v = float(fin[s][m]); row += f" {v:.4f} | {v-b:+.4f} |"
            else:
                row += " — | — |"
        out.append(row)

    # trayectorias por métrica
    for m in METRICAS:
        out.append(f"\n### {m}\n")
        out.append("| año | " + " | ".join(data.keys()) + " |")
        out.append("|" + "---|" * (len(data) + 1))
        for i, a in enumerate(anios):
            out.append(f"| {a} | " + " | ".join(f"{float(data[s][i][m]):.4f}" for s in data) + " |")

    (RES / "comparacion_escenarios.md").write_text("\n".join(out) + "\n", encoding="utf-8")
    print("OK ->", (RES / "comparacion_escenarios.md").relative_to(ROOT))

    # gráficas opcionales
    try:
        import matplotlib
        matplotlib.use("Agg")
        import matplotlib.pyplot as plt
        for m in METRICAS:
            plt.figure(figsize=(6, 4))
            for s in data:
                ys = [float(r[m]) for r in data[s]]
                plt.plot([int(a) for a in anios], ys, marker="o", label=s)
            plt.title(m); plt.xlabel("año"); plt.ylabel(m); plt.legend(); plt.grid(alpha=0.3)
            plt.tight_layout(); plt.savefig(RUNS / f"fig_{m}.png", dpi=110); plt.close()
        print("figuras ->", RUNS.relative_to(ROOT), "(fig_*.png)")
    except ImportError:
        print("(matplotlib no disponible; solo tabla markdown)")


if __name__ == "__main__":
    main()
