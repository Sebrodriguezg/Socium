#!/usr/bin/env python3
"""
Ensemble Monte Carlo con cuantificación de incertidumbre.

Para cada escenario corre R réplicas perturbando (a) la semilla estocástica y (b) los
parámetros inciertos (coeficientes de traducción de política y de calibración), y agrega
las trayectorias en media + banda predictiva p5-p95. Corre las réplicas en paralelo para
explotar el clúster (cada corrida usa --threads hilos; --jobs corridas simultáneas).

Salida:
  results/ensemble_<escenario>.csv   (anio, metrica, media, sd, p5, p95)
  results/ensemble_comparacion.md    (comparación final con bandas)

Uso (cluster):
  python3 scripts/ensemble.py --n 8000000 --reps 32 --jobs 8 --threads 16
"""
from __future__ import annotations
import argparse, csv, io, math, random, statistics, subprocess, tempfile, os
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
BIN = os.environ.get("SOCIUM_BIN", str(ROOT / "build" / "socium_simular"))
SCN = ROOT / "data" / "reference"
RES = ROOT / "results"
ESCENARIOS = ["baseline", "cepeda", "de_la_espriella"]
NOMBRE = {"baseline": "Continuidad", "cepeda": "Cepeda", "de_la_espriella": "De la Espriella"}
# parámetros globales inciertos perturbables por CLI (factor multiplicativo)
GLOBALES = ["--calib-ingreso", "--sigma", "--elast-pib", "--empleos-frac"]
DEFG = {"--calib-ingreso": 1.8, "--sigma": 0.80, "--elast-pib": 1.1, "--empleos-frac": 0.50}


def flat_perturbado(esc, pert, rng):
    """Copia el .flat del escenario escalando cada palanca por un factor aleatorio."""
    base = SCN / f"escenario_{esc}.flat"
    lineas = []
    for ln in base.read_text().splitlines():
        if ln.startswith("#") or not ln.strip():
            lineas.append(ln); continue
        k, v = ln.split()
        f = 1.0 + rng.uniform(-pert, pert)
        lineas.append(f"{k} {float(v)*f:.6g}")
    tmp = tempfile.NamedTemporaryFile("w", suffix=".flat", delete=False)
    tmp.write("\n".join(lineas)); tmp.close()
    return tmp.name


def una_corrida(esc, rep, args):
    rng = random.Random(hash((esc, rep)) & 0xffffffff)
    flat = flat_perturbado(esc, args.pert, rng)
    out = tempfile.NamedTemporaryFile("r", suffix=".csv", delete=False); out.close()
    cmd = [BIN, "--n", str(args.n), "--years", str(args.years), "--seed", str(1000 + rep),
           "--threads", str(args.threads), "--escenario", flat, "--out", out.name]
    # los parámetros de calibración están anclados a DANE -> perturbación pequeña (pert_calib)
    if args.pert_calib > 0:
        for g in GLOBALES:
            cmd += [g, f"{DEFG[g]*(1.0+rng.uniform(-args.pert_calib, args.pert_calib)):.6g}"]
    subprocess.run(cmd, capture_output=True)
    rows = list(csv.DictReader(open(out.name)))
    os.unlink(flat); os.unlink(out.name)
    return rows


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--n", type=int, default=3000000)
    ap.add_argument("--years", type=int, default=4)
    ap.add_argument("--reps", type=int, default=20)
    ap.add_argument("--jobs", type=int, default=4)
    ap.add_argument("--threads", type=int, default=4)
    ap.add_argument("--pert", type=float, default=0.20, help="perturbación de las palancas de política (incertidumbre de traducción)")
    ap.add_argument("--pert-calib", type=float, default=0.05, help="perturbación de params de calibración (anclados a DANE; pequeña)")
    args = ap.parse_args()
    if not Path(BIN).exists():
        raise SystemExit(f"No existe el binario {BIN} (compila core o exporta SOCIUM_BIN).")
    RES.mkdir(exist_ok=True)

    resumen = {}
    for esc in ESCENARIOS:
        print(f"[{esc}] {args.reps} réplicas (n={args.n}, jobs={args.jobs}×{args.threads} hilos, pert±{args.pert})...")
        with ThreadPoolExecutor(max_workers=args.jobs) as ex:
            corridas = list(ex.map(lambda r: una_corrida(esc, r, args), range(args.reps)))
        corridas = [c for c in corridas if c]
        anios = [r["anio"] for r in corridas[0]]
        metricas = [k for k in corridas[0][0].keys() if k not in ("anio", "poblacion")]
        agg = {}   # anio -> metrica -> [media, sd, p5, p95]
        with (RES / f"ensemble_{esc}.csv").open("w") as f:
            f.write("anio,metrica,media,sd,p5,p95\n")
            for i, anio in enumerate(anios):
                agg[anio] = {}
                for m in metricas:
                    vals = sorted(float(c[i][m]) for c in corridas)
                    media = statistics.fmean(vals)
                    sd = statistics.pstdev(vals)
                    p5 = vals[max(0, int(0.05*len(vals)))]
                    p95 = vals[min(len(vals)-1, int(0.95*len(vals)))]
                    agg[anio][m] = (media, sd, p5, p95)
                    f.write(f"{anio},{m},{media:.5f},{sd:.5f},{p5:.5f},{p95:.5f}\n")
        resumen[esc] = agg
        print(f"   -> results/ensemble_{esc}.csv")

    # comparación final con bandas
    anio_f = anios[-1]
    claves = [("pobreza","Pobreza"),("desempleo","Desempleo"),("gini_ingreso","Gini"),
              ("tasa_delincuencia","Delincuencia"),("satisfaccion_media","Satisfacción")]
    out = [f"# Ensemble — comparación {anio_f} (media [p5–p95], {args.reps} réplicas, pert ±{args.pert})\n",
           "| Métrica | " + " | ".join(NOMBRE[e] for e in ESCENARIOS) + " |",
           "|---|" + "---|"*len(ESCENARIOS)]
    for key, lab in claves:
        row = f"| {lab} |"
        for e in ESCENARIOS:
            me, sd, p5, p95 = resumen[e][anio_f][key]
            esc100 = key != "satisfaccion_media"
            f = (lambda x: f"{x*100:.1f}%") if esc100 else (lambda x: f"{x:.2f}")
            row += f" {f(me)} [{f(p5)}–{f(p95)}] |"
        out.append(row)
    (RES / "ensemble_comparacion.md").write_text("\n".join(out)+"\n", encoding="utf-8")
    print("->", (RES/"ensemble_comparacion.md").relative_to(ROOT))
    print("\n".join(out))


if __name__ == "__main__":
    main()
