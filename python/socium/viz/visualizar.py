#!/usr/bin/env python3
"""
Genera gráficos SVG (sin dependencias) de las trayectorias 2026->2030 por escenario,
para visualizar la comparación de programas de gobierno.

Lee experiments/runs/serie_<id>.csv y escribe results/serie_<metrica>.svg

    python3 python/socium/viz/visualizar.py
"""
from __future__ import annotations
import csv
from pathlib import Path

ROOT = Path(__file__).resolve().parents[3]
RUNS = ROOT / "experiments" / "runs"
RES = ROOT / "results"
ESC = {"baseline": "#888888", "cepeda": "#E94560", "de_la_espriella": "#0F3460"}
METRICAS = {
    "pobreza": "Pobreza monetaria",
    "desempleo": "Desempleo",
    "tasa_delincuencia": "Tasa de delincuencia",
    "satisfaccion_media": "Satisfacción con la vida (0-10)",
    "deuda_pib": "Deuda pública (% del PIB)",
    "deficit_pib": "Déficit fiscal (% del PIB)",
}


def cargar():
    d = {}
    for s in ESC:
        f = RUNS / f"serie_{s}.csv"
        if f.exists():
            d[s] = list(csv.DictReader(open(f)))
    return d


def line_chart(data, metric, title, path):
    W, H, m = 620, 400, 64
    anios = [int(r["anio"]) for r in next(iter(data.values()))]
    vals = [float(r[metric]) for s in data for r in data[s]]
    lo, hi = min(vals), max(vals)
    if metric == "satisfaccion_media": lo, hi = 0, 10
    else: lo, hi = max(0, lo - 0.02), hi + 0.02
    if hi <= lo: hi = lo + 1
    def sx(a): return m + (a - anios[0]) / max(1, anios[-1] - anios[0]) * (W - 2*m)
    def sy(v): return H - m - (v - lo) / (hi - lo) * (H - 2*m)
    s = [f'<svg xmlns="http://www.w3.org/2000/svg" width="{W}" height="{H}" font-family="Arial">',
         f'<rect width="{W}" height="{H}" fill="white"/>',
         f'<text x="{W/2}" y="26" font-size="16" font-weight="bold" text-anchor="middle">{title}</text>',
         f'<line x1="{m}" y1="{H-m}" x2="{W-m}" y2="{H-m}" stroke="#333"/>',
         f'<line x1="{m}" y1="{m}" x2="{m}" y2="{H-m}" stroke="#333"/>']
    for k in range(5):
        v = lo + (hi - lo) * k / 4
        lbl = f"{v*100:.0f}%" if metric != "satisfaccion_media" else f"{v:.1f}"
        s.append(f'<text x="{m-10}" y="{sy(v)+4}" font-size="11" text-anchor="end">{lbl}</text>')
        s.append(f'<line x1="{m}" y1="{sy(v)}" x2="{W-m}" y2="{sy(v)}" stroke="#eee"/>')
    for a in anios:
        s.append(f'<text x="{sx(a)}" y="{H-m+18}" font-size="11" text-anchor="middle">{a}</text>')
    for si, (esc, rows) in enumerate(data.items()):
        pts = " ".join(f"{sx(int(r['anio'])):.1f},{sy(float(r[metric])):.1f}" for r in rows)
        s.append(f'<polyline points="{pts}" fill="none" stroke="{ESC[esc]}" stroke-width="2.5"/>')
        for r in rows:
            s.append(f'<circle cx="{sx(int(r["anio"])):.1f}" cy="{sy(float(r[metric])):.1f}" r="3" fill="{ESC[esc]}"/>')
        s.append(f'<text x="{W-m+6}" y="{38+si*18}" font-size="12" fill="{ESC[esc]}">{esc}</text>')
    s.append('</svg>')
    path.write_text("\n".join(s), encoding="utf-8")


def main():
    data = cargar()
    if not data:
        raise SystemExit("Faltan series. Corre socium_simular --escenario ... --out ... primero.")
    RES.mkdir(exist_ok=True)
    for met, tit in METRICAS.items():
        line_chart(data, met, tit, RES / f"serie_{met}.svg")
        print("->", (RES / f"serie_{met}.svg").relative_to(ROOT))


if __name__ == "__main__":
    main()
