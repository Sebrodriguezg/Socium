#!/usr/bin/env python3
"""
Descarga el PIB departamental (DANE, datos.gov.co kgyi-qc7j), suma todas las actividades
del año más reciente y calcula el PIB per cápita por departamento (usando la población de
poblacion_municipal.csv). Sirve como driver de PRODUCTIVIDAD REGIONAL para descomprimir
la desigualdad espacial de ingreso del modelo.

Salida: data/reference/pib_departamental.csv
Columnas: cod_dpto, pib_pc_cop, ratio (PIB pc / PIB pc nacional)

    python3 scripts/fetch_pib_departamental.py [anio]   # por defecto 2023
"""
from __future__ import annotations
import csv, json, sys, urllib.parse, urllib.request
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
POB = ROOT / "data" / "reference" / "poblacion_municipal.csv"
OUT = ROOT / "data" / "reference" / "pib_departamental.csv"
DATASET = "kgyi-qc7j"
ANIO = sys.argv[1] if len(sys.argv) > 1 else "2023"


def fetch_pib():
    where = urllib.parse.quote(f"a_o='{ANIO}' AND tipo_de_precios='PIB a precios corrientes'")
    sel = urllib.parse.quote("c_digo_departamento_divipola,sum(valor_miles_de_millones_de)")
    url = (f"https://www.datos.gov.co/resource/{DATASET}.json?"
           f"$select={sel}&$where={where}&$group=c_digo_departamento_divipola&$limit=200")
    req = urllib.request.Request(url, headers={"User-Agent": "Socium/0.1"})
    with urllib.request.urlopen(req, timeout=60) as r:
        return json.loads(r.read().decode("utf-8"))


def poblacion_por_depto():
    pob = {}
    with POB.open(encoding="utf-8") as f:
        for row in csv.DictReader(f):
            d = row["cod_mpio"][:2].zfill(2)
            try: pob[d] = pob.get(d, 0) + int(row["poblacion_total"])
            except ValueError: pass
    return pob


def main():
    pob = poblacion_por_depto()
    data = fetch_pib()
    pibpc = {}
    for r in data:
        d = str(r.get("c_digo_departamento_divipola", "")).zfill(2)
        s = r.get("sum_valor_miles_de_millones_de")
        if not s or d not in pob or pob[d] <= 0:
            continue
        pib_cop = float(s) * 1e9            # miles de millones -> COP
        pibpc[d] = pib_cop / pob[d]         # COP per cápita / año

    if not pibpc:
        sys.exit("Sin datos de PIB (revisar API/filtros).")
    # PIB pc nacional ponderado
    pib_total = sum(pibpc[d] * pob[d] for d in pibpc)
    pob_total = sum(pob[d] for d in pibpc)
    nac = pib_total / pob_total

    OUT.parent.mkdir(parents=True, exist_ok=True)
    with OUT.open("w", encoding="utf-8") as f:
        f.write("cod_dpto,pib_pc_cop,ratio\n")
        for d in sorted(pibpc):
            f.write(f"{d},{pibpc[d]:.0f},{pibpc[d]/nac:.4f}\n")

    rs = sorted(((pibpc[d]/nac, d) for d in pibpc), reverse=True)
    print(f"OK -> {OUT.relative_to(ROOT)}  (año {ANIO}, {len(pibpc)} deptos, PIB pc nacional {nac/1e6:.1f}M COP)")
    print("   top ratio:", [(d, round(r,2)) for r, d in rs[:3]], " bottom:", [(d, round(r,2)) for r, d in rs[-3:]])


if __name__ == "__main__":
    main()
