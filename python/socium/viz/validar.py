#!/usr/bin/env python3
"""
Validación espacial: compara la pobreza por departamento SIMULADA (modelo) contra la
REAL (DANE 2023). Calcula correlaciones (Pearson, Spearman) y MAE, y genera:
  - results/validacion.md           (reporte)
  - results/validacion_pobreza.svg  (scatter simulado vs real, sin dependencias)

Requiere haber corrido:  socium_simular ... --out-deptos experiments/runs/deptos_baseline.csv

    python3 python/socium/viz/validar.py
"""
from __future__ import annotations
import csv, math, unicodedata
from pathlib import Path

ROOT = Path(__file__).resolve().parents[3]
SIM = ROOT / "experiments" / "runs" / "deptos_baseline.csv"
REAL = ROOT / "data" / "reference" / "pobreza_departamental_dane2023.csv"
RES = ROOT / "results"


def norm(s):
    s = unicodedata.normalize("NFKD", s).encode("ascii", "ignore").decode().upper().strip()
    return s


def pearson(x, y):
    n = len(x); mx = sum(x)/n; my = sum(y)/n
    sxy = sum((a-mx)*(b-my) for a, b in zip(x, y))
    sx = math.sqrt(sum((a-mx)**2 for a in x)); sy = math.sqrt(sum((b-my)**2 for b in y))
    return sxy/(sx*sy) if sx*sy else 0.0


def spearman(x, y):
    def ranks(v):
        order = sorted(range(len(v)), key=lambda i: v[i])
        r = [0]*len(v)
        for rank, i in enumerate(order): r[i] = rank
        return r
    return pearson(ranks(x), ranks(y))


def svg_scatter(pts, labels, path, title):
    W, H, m = 560, 520, 70
    lo, hi = 0.15, 0.75
    def sx(v): return m + (v-lo)/(hi-lo)*(W-2*m)
    def sy(v): return H-m - (v-lo)/(hi-lo)*(H-2*m)
    s = [f'<svg xmlns="http://www.w3.org/2000/svg" width="{W}" height="{H}" font-family="Arial">']
    s.append(f'<rect width="{W}" height="{H}" fill="white"/>')
    s.append(f'<text x="{W/2}" y="28" font-size="16" font-weight="bold" text-anchor="middle">{title}</text>')
    # diagonal y=x
    s.append(f'<line x1="{sx(lo)}" y1="{sy(lo)}" x2="{sx(hi)}" y2="{sy(hi)}" stroke="#bbb" stroke-dasharray="5,4"/>')
    # ejes
    s.append(f'<line x1="{m}" y1="{H-m}" x2="{W-m}" y2="{H-m}" stroke="#333"/>')
    s.append(f'<line x1="{m}" y1="{m}" x2="{m}" y2="{H-m}" stroke="#333"/>')
    for t in [0.2, 0.4, 0.6]:
        s.append(f'<text x="{sx(t)}" y="{H-m+18}" font-size="11" text-anchor="middle">{int(t*100)}%</text>')
        s.append(f'<text x="{m-22}" y="{sy(t)+4}" font-size="11" text-anchor="middle">{int(t*100)}%</text>')
    s.append(f'<text x="{W/2}" y="{H-20}" font-size="13" text-anchor="middle">Real DANE 2023 (pobreza)</text>')
    s.append(f'<text x="20" y="{H/2}" font-size="13" text-anchor="middle" transform="rotate(-90 20 {H/2})">Modelo (pobreza)</text>')
    for (xr, ys), lab in zip(pts, labels):
        s.append(f'<circle cx="{sx(xr):.1f}" cy="{sy(ys):.1f}" r="4.5" fill="#E94560"/>')
        s.append(f'<text x="{sx(xr)+6:.1f}" y="{sy(ys)-4:.1f}" font-size="9" fill="#444">{lab[:9]}</text>')
    s.append('</svg>')
    path.write_text("\n".join(s), encoding="utf-8")


def main():
    if not SIM.exists():
        raise SystemExit(f"Falta {SIM}. Corre socium_simular --out-deptos ... primero.")
    sim = {r["cod_dpto"].zfill(2): float(r["pobreza"]) for r in csv.DictReader(open(SIM))}
    real_rows = list(csv.DictReader(open(REAL)))
    xr, ys, labs = [], [], []
    for r in real_rows:
        code = r["cod_dpto"].zfill(2)
        if code in sim:
            xr.append(float(r["pobreza_real"])); ys.append(sim[code]); labs.append(r["departamento"])

    pr = pearson(xr, ys); sp = spearman(xr, ys)
    mae = sum(abs(a-b) for a, b in zip(xr, ys))/len(xr)
    RES.mkdir(exist_ok=True)
    svg_scatter(list(zip(xr, ys)), labs, RES/"validacion_pobreza.svg",
                "Validación pobreza por departamento")

    out = ["# Validación espacial — pobreza por departamento\n",
           "Modelo (baseline 2026) vs DANE 2023, por departamento.\n",
           f"- Departamentos comparados: **{len(xr)}**",
           f"- Correlación de Pearson: **{pr:.3f}**",
           f"- Correlación de Spearman (orden): **{sp:.3f}**",
           f"- Error absoluto medio (MAE): **{mae*100:.1f} pp**",
           f"- Rango real: {min(xr)*100:.0f}%–{max(xr)*100:.0f}%  ·  Rango modelo: {min(ys)*100:.0f}%–{max(ys)*100:.0f}%\n",
           "| Departamento | Real DANE | Modelo | Δ |",
           "|---|---|---|---|"]
    for r, s_, l in sorted(zip(xr, ys, labs), reverse=True):
        out.append(f"| {l} | {r*100:.1f}% | {s_*100:.1f}% | {(s_-r)*100:+.1f} pp |")
    out += ["\n**Lectura:** una Spearman alta indica que el modelo ordena bien los "
            "departamentos (reproduce el patrón espacial); un MAE/rango menor que el real "
            "indica que el modelo **comprime** la desigualdad espacial (driver estructural "
            "regional aún por calibrar más allá de educación rural y conflicto).",
            "\n![scatter](validacion_pobreza.svg)"]
    (RES/"validacion.md").write_text("\n".join(out)+"\n", encoding="utf-8")
    print(f"Pearson={pr:.3f}  Spearman={sp:.3f}  MAE={mae*100:.1f}pp  (n={len(xr)})")
    print("->", (RES/"validacion.md").relative_to(ROOT), "y validacion_pobreza.svg")


if __name__ == "__main__":
    main()
