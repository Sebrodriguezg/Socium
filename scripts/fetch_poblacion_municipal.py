#!/usr/bin/env python3
"""
Descarga estadísticas municipales del MEN (datos.gov.co, dataset nudc-7mev), que
tienen cobertura nacional con código DANE de municipio, y las une con DIVIPOLA.

Genera data/reference/municipios.csv con, por municipio:
  cod_mpio, poblacion_5_16, cobertura_neta, cobertura_bruta, desercion

`poblacion_5_16` (población en edad escolar 5-16) se usa como **proxy del tamaño
poblacional** para distribuir espacialmente a los agentes, hasta integrar la
población total oficial. Las columnas de educación alimentan el dominio educativo.

    python3 scripts/fetch_poblacion_municipal.py [anio]   # por defecto 2024
"""
from __future__ import annotations
import csv, json, sys, urllib.parse, urllib.request
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
DIVIPOLA = ROOT / "data" / "reference" / "divipola.csv"
OUT = ROOT / "data" / "reference" / "municipios.csv"
DATASET = "nudc-7mev"
ANIO = sys.argv[1] if len(sys.argv) > 1 else "2024"


def fetch():
    cols = ("c_digo_municipio,municipio,poblaci_n_5_16,"
            "cobertura_neta,cobertura_bruta,deserci_n")
    url = (f"https://www.datos.gov.co/resource/{DATASET}.json?"
           f"a_o={ANIO}&$select={urllib.parse.quote(cols)}&$limit=5000")
    req = urllib.request.Request(url, headers={"User-Agent": "Socium/0.1"})
    with urllib.request.urlopen(req, timeout=90) as r:
        return json.loads(r.read().decode("utf-8"))


def load_divipola_codes() -> set[str]:
    with DIVIPOLA.open(encoding="utf-8", newline="") as f:
        return {row["cod_mpio"].strip() for row in csv.DictReader(f)}


def main():
    if not DIVIPOLA.exists():
        sys.exit("Falta data/reference/divipola.csv (corre fetch_divipola.py primero)")
    valid = load_divipola_codes()
    data = fetch()

    rows, matched = [], 0
    for r in data:
        code = str(r.get("c_digo_municipio", "")).strip()
        if not code.isdigit():
            continue
        code = code.zfill(5)                       # normaliza a 5 dígitos DANE
        if code not in valid:                      # solo municipios reales (no ETC/agregados)
            continue
        rows.append((code,
                     r.get("poblaci_n_5_16", ""),
                     r.get("cobertura_neta", ""),
                     r.get("cobertura_bruta", ""),
                     r.get("deserci_n", "")))
        matched += 1

    rows.sort()
    OUT.parent.mkdir(parents=True, exist_ok=True)
    with OUT.open("w", encoding="utf-8") as f:
        f.write("cod_mpio,poblacion_5_16,cobertura_neta,cobertura_bruta,desercion\n")
        for c, p, cn, cb, d in rows:
            f.write(f"{c},{p},{cn},{cb},{d}\n")

    print(f"OK -> {OUT.relative_to(ROOT)}  (año {ANIO}, {matched} municipios unidos a DIVIPOLA "
          f"de {len(valid)} entidades)")


if __name__ == "__main__":
    main()
