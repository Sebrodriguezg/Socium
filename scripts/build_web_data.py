#!/usr/bin/env python3
"""
Empaqueta los perfiles de los 3 escenarios en un JSON compacto para la web pública
(consulta en el navegador, sin servidor -> hospedable en GitHub Pages).

Lee experiments/runs/perfiles_<escenario>.csv y escribe:
  web/data/perfiles.json       (clave "cod|sexo|edad|educ" -> escenario -> año -> métricas)
  web/data/departamentos.json  (cod -> nombre, para el menú)

    python3 scripts/build_web_data.py
"""
from __future__ import annotations
import csv, json
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
RUNS = ROOT / "experiments" / "runs"
DIVIPOLA = ROOT / "data" / "reference" / "divipola.csv"
WEBDATA = ROOT / "web" / "data"
ESCENARIOS = ["baseline", "cepeda", "de_la_espriella"]
# orden de métricas almacenadas (compacto, como lista)
MET = ["ingreso_pc", "p_ocupado", "p_pobreza", "p_enfermo", "p_delincuencia", "satisfaccion"]


def main():
    WEBDATA.mkdir(parents=True, exist_ok=True)
    perfiles: dict = {}
    for esc in ESCENARIOS:
        f = RUNS / f"perfiles_{esc}.csv"
        if not f.exists():
            raise SystemExit(f"Falta {f}. Corre socium_simular --out-perfiles primero.")
        for r in csv.DictReader(open(f)):
            key = f"{r['cod_dpto']}|{r['sexo']}|{r['edad']}|{r['educacion']}|{r['situacion']}"
            d = perfiles.setdefault(key, {})
            de = d.setdefault(esc, {})
            de[r["anio"]] = [round(float(r[m]), 4) for m in MET]

    (WEBDATA / "perfiles.json").write_text(
        json.dumps({"metricas": MET, "escenarios": ESCENARIOS, "datos": perfiles},
                   ensure_ascii=False, separators=(",", ":")), encoding="utf-8")

    deptos = {}
    for r in csv.DictReader(open(DIVIPOLA)):
        deptos[r["cod_dpto"]] = r["dpto"]
    (WEBDATA / "departamentos.json").write_text(
        json.dumps(deptos, ensure_ascii=False), encoding="utf-8")

    # --- datos del mapa: agrega perfiles a nivel departamento (ponderado por n) ---
    mapa = {}   # esc -> anio -> cod_dpto -> [sum_n, sum_m0, sum_m1, ...]
    for esc in ESCENARIOS:
        for r in csv.DictReader(open(RUNS / f"perfiles_{esc}.csv")):
            n = float(r["n"]); cod = r["cod_dpto"]; anio = r["anio"]
            cell = mapa.setdefault(esc, {}).setdefault(anio, {}).setdefault(cod, [0.0] + [0.0]*len(MET))
            cell[0] += n
            for j, m in enumerate(MET):
                cell[j+1] += float(r[m]) * n
    salida = {}   # esc -> anio -> cod -> [promedios]
    for esc, ys in mapa.items():
        for anio, cods in ys.items():
            for cod, acc in cods.items():
                n = acc[0] or 1
                salida.setdefault(esc, {}).setdefault(anio, {})[cod] = [round(acc[j+1]/n, 4) for j in range(len(MET))]
    (WEBDATA / "mapa.json").write_text(
        json.dumps({"metricas": MET, "escenarios": ESCENARIOS, "datos": salida},
                   ensure_ascii=False, separators=(",", ":")), encoding="utf-8")

    size = (WEBDATA / "perfiles.json").stat().st_size / 1024
    print(f"OK -> web/data/perfiles.json ({size:.0f} KB, {len(perfiles)} perfiles), departamentos.json ({len(deptos)})")

    # --- muestra de agentes para consulta multi-filtro (snapshot año final) ---
    mfile = RUNS / "muestra_baseline.csv"
    if mfile.exists():
        ESC = ["baseline", "cepeda", "de_la_espriella"]
        campos = None; muestra = {}
        for esc in ESC:
            f = RUNS / f"muestra_{esc}.csv"
            if not f.exists(): continue
            rd = csv.reader(open(f)); campos = next(rd)
            ii = campos.index("ingreso_pc")
            filas = []
            for row in rd:
                v = [int(x) for x in row]
                v[ii] = v[ii] // 1000          # ingreso en miles de COP (compacto)
                filas.append(v)
            muestra[esc] = filas
        (WEBDATA / "muestra.json").write_text(
            json.dumps({"campos": campos, "escenarios": ESC, "datos": muestra},
                       separators=(",", ":")), encoding="utf-8")
        mb = (WEBDATA / "muestra.json").stat().st_size / 1024
        print(f"OK -> web/data/muestra.json ({mb:.0f} KB, {len(muestra.get('baseline',[]))} agentes/escenario)")


if __name__ == "__main__":
    main()
