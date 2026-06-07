#!/usr/bin/env python3
"""
Descarga homicidios por municipio (Policía Nacional, datos.gov.co m8fd-ahd9),
agrega los años recientes, los une con población (poblacion_municipal.csv) y calcula
la tasa de homicidios por 100k y un índice de conflicto normalizado [0,1].

Sirve como: (a) ancla de calibración de la delincuencia (M5, spec §6.4), y
(b) proxy de presencia de grupos armados / intensidad de conflicto por municipio.

Salida: data/reference/conflicto_municipal.csv
Columnas: cod_mpio, homicidios_anual, tasa_homicidios_100k, indice_conflicto

    python3 scripts/fetch_conflicto.py [anio_desde]   # por defecto 2022
"""
from __future__ import annotations
import csv, json, sys, urllib.parse, urllib.request
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
POB = ROOT / "data" / "reference" / "poblacion_municipal.csv"
OUT = ROOT / "data" / "reference" / "conflicto_municipal.csv"
DATASET = "m8fd-ahd9"
DESDE = sys.argv[1] if len(sys.argv) > 1 else "2022"


def fetch_homicidios():
    where = urllib.parse.quote(f"fecha_hecho>='{DESDE}-01-01'")
    sel = urllib.parse.quote("cod_muni,sum(cantidad)")
    url = (f"https://www.datos.gov.co/resource/{DATASET}.json?"
           f"$select={sel}&$where={where}&$group=cod_muni&$limit=5000")
    req = urllib.request.Request(url, headers={"User-Agent": "Socium/0.1"})
    with urllib.request.urlopen(req, timeout=90) as r:
        return json.loads(r.read().decode("utf-8"))


def rango_anios():
    where = urllib.parse.quote(f"fecha_hecho>='{DESDE}-01-01'")
    url = (f"https://www.datos.gov.co/resource/{DATASET}.json?"
           f"$select=min(fecha_hecho),max(fecha_hecho)&$where={where}")
    req = urllib.request.Request(url, headers={"User-Agent": "Socium/0.1"})
    with urllib.request.urlopen(req, timeout=60) as r:
        d = json.loads(r.read().decode("utf-8"))[0]
    y0 = int(d["min_fecha_hecho"][:4]) + (int(d["min_fecha_hecho"][5:7]) - 1) / 12.0
    y1 = int(d["max_fecha_hecho"][:4]) + (int(d["max_fecha_hecho"][5:7]) - 1) / 12.0
    return max(1.0, y1 - y0)


def main():
    pob = {}
    with POB.open(encoding="utf-8") as f:
        for row in csv.DictReader(f):
            try: pob[row["cod_mpio"]] = int(row["poblacion_total"])
            except (ValueError, KeyError): pass

    n_anios = rango_anios()             # del rango real de fechas
    data = fetch_homicidios()
    filas = {}
    for r in data:
        code = str(r.get("cod_muni", "")).strip().zfill(5)
        s = r.get("sum_cantidad") or r.get("sum_cantidad".replace("_", " "))
        try: total = float(s)
        except (TypeError, ValueError): continue
        if code not in pob or pob[code] <= 0:
            continue
        anual = total / n_anios
        tasa = 100000.0 * anual / pob[code]
        filas[code] = (anual, tasa)

    if not filas:
        sys.exit("Sin datos agregados (revisar API).")
    tmax = max(t for _, t in filas.values()) or 1.0

    OUT.parent.mkdir(parents=True, exist_ok=True)
    with OUT.open("w", encoding="utf-8") as f:
        f.write("cod_mpio,homicidios_anual,tasa_homicidios_100k,indice_conflicto\n")
        for code in sorted(filas):
            anual, tasa = filas[code]
            idx = min(1.0, tasa / 80.0)   # normaliza: 80/100k -> 1.0 (zona muy violenta)
            f.write(f"{code},{anual:.1f},{tasa:.2f},{idx:.4f}\n")

    tasas = sorted((t for _, t in filas.values()), reverse=True)
    print(f"OK -> {OUT.relative_to(ROOT)}  ({len(filas)} municipios, desde {DESDE})")
    print(f"   tasa nacional aprox: {sum(a for a,_ in filas.values())*100000/sum(pob[c] for c in filas):.1f}/100k")
    print(f"   top tasas/100k: {[round(t,1) for t in tasas[:5]]}")


if __name__ == "__main__":
    main()
