#!/usr/bin/env python3
"""
Validación FUERA DE MUESTRA: simula 2018->2023 (con shock exógeno COVID-2020) y compara
contra la serie histórica real (Banco Mundial / DANE). Calcula MAE por indicador (y MAE
sin sesgo, que mide el ajuste de FORMA) y genera un overlay SVG real vs simulado.

    python3 python/socium/viz/validar_historico.py [shock_mag]
"""
from __future__ import annotations
import csv, os, subprocess, sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[3]
BIN = os.environ.get("SOCIUM_BIN", str(ROOT / "build" / "socium_simular"))
HIST = ROOT / "data" / "reference" / "historico_nacional.csv"
FLAT = ROOT / "data" / "reference" / "escenario_baseline.flat"
RES = ROOT / "results"
SHOCK = float(sys.argv[1]) if len(sys.argv) > 1 else 0.15
INDS = [("pobreza", "Pobreza"), ("desempleo", "Desempleo"), ("gini_ingreso", "Gini")]
HCOL = {"pobreza": "pobreza", "desempleo": "desempleo", "gini_ingreso": "gini"}


def simular():
    out = "/tmp/hist_sim.csv"
    subprocess.run([BIN, "--n", "3000000", "--anio-inicial", "2018", "--years", "5",
                    "--shock-anio", "2020", "--shock-mag", str(SHOCK), "--seed", "7",
                    "--escenario", str(FLAT), "--out", out], capture_output=True)
    return {r["anio"]: r for r in csv.DictReader(open(out))}


def mae(xs, ys):
    return sum(abs(a-b) for a, b in zip(xs, ys))/len(xs)


def svg(real, sim, anios, key, lab, path):
    W, H, m = 560, 360, 56
    allv = real + sim
    lo, hi = min(allv)*0.9, max(allv)*1.1
    def sx(a): return m + (a-anios[0])/(anios[-1]-anios[0])*(W-2*m)
    def sy(v): return H-m - (v-lo)/(hi-lo)*(H-2*m)
    s=[f'<svg xmlns="http://www.w3.org/2000/svg" width="{W}" height="{H}" font-family="Arial">',
       f'<rect width="{W}" height="{H}" fill="white"/>',
       f'<text x="{W/2}" y="24" font-size="15" font-weight="bold" text-anchor="middle">{lab}: real vs simulado 2018–2023</text>',
       f'<line x1="{m}" y1="{H-m}" x2="{W-m}" y2="{H-m}" stroke="#333"/><line x1="{m}" y1="{m}" x2="{m}" y2="{H-m}" stroke="#333"/>']
    for a in anios: s.append(f'<text x="{sx(a)}" y="{H-m+16}" font-size="10" text-anchor="middle">{a}</text>')
    for k in range(5):
        v=lo+(hi-lo)*k/4
        s.append(f'<text x="{m-8}" y="{sy(v)+3}" font-size="9" text-anchor="end">{v*100:.0f}%</text>')
    def poly(vals,color,dash):
        p=" ".join(f"{sx(a):.0f},{sy(v):.0f}" for a,v in zip(anios,vals))
        return f'<polyline points="{p}" fill="none" stroke="{color}" stroke-width="2.5" {dash}/>'
    s.append(poly(real,"#0F3460",""))
    s.append(poly(sim,"#E94560",'stroke-dasharray="6,4"'))
    s.append(f'<text x="{W-m-110}" y="{m+6}" font-size="12" fill="#0F3460">— real (DANE/BM)</text>')
    s.append(f'<text x="{W-m-110}" y="{m+24}" font-size="12" fill="#E94560">- - simulado</text>')
    s.append('</svg>')
    path.write_text("\n".join(s),encoding="utf-8")


def main():
    if not Path(BIN).exists(): raise SystemExit(f"No existe {BIN}")
    real_rows = {r["anio"]: r for r in csv.DictReader(open(HIST))}
    anios = sorted(int(a) for a in real_rows)
    sim = simular()
    RES.mkdir(exist_ok=True)
    out = [f"# Validación fuera de muestra 2018–2023 (shock COVID-2020 mag={SHOCK})\n",
           "Modelo iniciado en 2018, corrido a 2023 con un shock exógeno en 2020 (la pandemia "
           "no es endógena a ningún modelo). Comparado con la serie real (Banco Mundial/DANE).\n",
           "| Indicador | MAE | MAE sin sesgo (forma) |", "|---|---|---|"]
    for key, lab in INDS:
        r = [float(real_rows[str(a)][HCOL[key]]) for a in anios]
        sm = [float(sim[str(a)][key]) for a in anios]
        bias = sum(s-x for s, x in zip(sm, r))/len(r)
        m_raw = mae(r, sm); m_form = mae(r, [s-bias for s in sm])
        out.append(f"| {lab} | {m_raw*100:.1f} pp | {m_form*100:.1f} pp |")
        if key in ("pobreza", "desempleo"):
            svg(r, sm, anios, key, lab, RES/f"validacion_hist_{key}.svg")
    out += ["\n**Lectura:** el MAE sin sesgo mide si el modelo reproduce la **forma** (caída "
            "por COVID y recuperación). Un sesgo constante refleja la diferencia de nivel entre "
            "nuestra calibración (~32%) y la cifra revisada de pobreza (BM 34.6%). El modelo capta "
            "niveles y la dirección del shock; la magnitud del pico de pobreza queda amortiguada "
            "(colchón de ingreso no laboral) y la recuperación del empleo es lenta (ciclo de primer "
            "orden) — a mejorar en Tier 2 (colchón de fuerza laboral, dinámica de recuperación).",
            "\n![pobreza](validacion_hist_pobreza.svg)\n\n![desempleo](validacion_hist_desempleo.svg)"]
    (RES/"validacion_historica.md").write_text("\n".join(out)+"\n", encoding="utf-8")
    print("\n".join(out[:10]))
    print("->", (RES/"validacion_historica.md").relative_to(ROOT))


if __name__ == "__main__":
    main()
