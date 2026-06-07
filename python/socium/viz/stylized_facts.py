#!/usr/bin/env python3
"""
Verifica que el modelo reproduce (sin forzar) los HECHOS ESTILIZADOS que un ABM macro de
referencia debe cumplir: concentración del ingreso (top 10% ~40%), distribución de tamaño
de empresas dominada por microempresas (~92%) tipo Zipf, y Gini coherente.

Requiere: socium_simular ... --out-distribucion experiments/runs/distribucion.csv

    python3 python/socium/viz/stylized_facts.py
"""
from __future__ import annotations
import csv, math, os, subprocess
from pathlib import Path

ROOT = Path(__file__).resolve().parents[3]
BIN = os.environ.get("SOCIUM_BIN", str(ROOT / "build" / "socium_simular"))
RES = ROOT / "results"
DIST = ROOT / "experiments" / "runs" / "distribucion.csv"


def run():
    DIST.parent.mkdir(parents=True, exist_ok=True)
    subprocess.run([BIN, "--n", "5000000", "--years", "4", "--seed", "7",
                    "--escenario", str(ROOT/"data/reference/escenario_baseline.flat"),
                    "--out", "/dev/null", "--out-distribucion", str(DIST)], capture_output=True)


def main():
    if not DIST.exists():
        if not Path(BIN).exists(): raise SystemExit(f"No existe {BIN}")
        run()
    d = {}
    for r in csv.DictReader(open(DIST)):
        d.setdefault(r["tipo"], {})[r["clave"]] = float(r["valor"])

    top10 = d.get("top_share", {}).get("top10", 0)
    top1 = d.get("top_share", {}).get("top1", 0)
    deciles = [d["decil_ingreso"][str(i)] for i in range(1, 11)]
    micro = d.get("empresa_tamano", {}).get("micro", 0)
    grande = d.get("empresa_tamano", {}).get("grande", 0)
    # Gini desde la curva de Lorenz (deciles)
    cum = 0; lor = []
    for s in deciles: cum += s; lor.append(cum)
    gini = 1 - sum((lor[i]+ (lor[i-1] if i else 0)) for i in range(10))/10

    def check(nombre, val, lo, hi, fmt="{:.1%}"):
        ok = lo <= val <= hi
        return f"| {nombre} | {fmt.format(val)} | {fmt.format(lo)}–{fmt.format(hi)} | {'✅' if ok else '⚠️'} |", ok

    filas, oks = [], []
    for args in [
        ("Top 10% del ingreso", top10, 0.35, 0.45),
        ("Top 1% del ingreso", top1, 0.08, 0.22),
        ("Gini (curva de Lorenz)", gini, 0.50, 0.60),
        ("Microempresas (% del total)", micro, 0.85, 0.95),
        ("Grandes empresas (% del total)", grande, 0.005, 0.03),
    ]:
        f, ok = check(*args); filas.append(f); oks.append(ok)

    out = ["# Hechos estilizados — verificación\n",
           "Propiedades macro que un ABM competitivo debe reproducir COMO EMERGENCIA "
           "(no impuestas). Rangos de referencia: Colombia/DANE y literatura.\n",
           "| Hecho estilizado | Modelo | Referencia | |", "|---|---|---|---|", *filas,
           f"\n**{sum(oks)}/{len(oks)} hechos estilizados dentro de rango.**",
           "\nDistribución de tamaño de empresas (debe decaer tipo Zipf, micro-dominada):",
           "| micro | pequeña | mediana | grande |", "|---|---|---|---|",
           "| {:.1%} | {:.1%} | {:.1%} | {:.1%} |".format(
               d["empresa_tamano"]["micro"], d["empresa_tamano"]["pequena"],
               d["empresa_tamano"]["mediana"], d["empresa_tamano"]["grande"]),
           "\nParticipación del ingreso por decil (D1 más pobre → D10 más rico):",
           "`" + "  ".join(f"D{i+1}:{deciles[i]*100:.1f}%" for i in range(10)) + "`"]
    RES.mkdir(exist_ok=True)
    (RES/"stylized_facts.md").write_text("\n".join(out)+"\n", encoding="utf-8")
    print("\n".join(out))
    print("\n->", (RES/"stylized_facts.md").relative_to(ROOT))


if __name__ == "__main__":
    main()
