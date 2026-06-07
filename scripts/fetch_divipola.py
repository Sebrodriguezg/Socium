#!/usr/bin/env python3
"""
Descarga la división político-administrativa de Colombia (DIVIPOLA, DANE) desde
datos.gov.co y la normaliza a un CSV de referencia versionado.

Salida: data/reference/divipola.csv
Columnas: cod_dpto, dpto, cod_mpio, nom_mpio, tipo, lon, lat

Reproducible: re-ejecutar actualiza el CSV. Sin dependencias externas.

    python3 scripts/fetch_divipola.py
"""
from __future__ import annotations
import json, sys, urllib.request
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
OUT = ROOT / "data" / "reference" / "divipola.csv"
URL = "https://www.datos.gov.co/resource/gdxc-w37w.json?$limit=5000"


def num(s: str) -> str:
    s = (s or "").strip().replace(",", ".")
    try:
        return repr(float(s))
    except ValueError:
        return ""


def csv_cell(s: str) -> str:
    s = (s or "").strip()
    return '"' + s.replace('"', '""') + '"' if ("," in s or '"' in s) else s


def main():
    req = urllib.request.Request(URL, headers={"User-Agent": "Socium/0.1"})
    with urllib.request.urlopen(req, timeout=60) as r:
        data = json.loads(r.read().decode("utf-8"))

    rows = sorted(data, key=lambda x: x.get("cod_mpio", ""))
    OUT.parent.mkdir(parents=True, exist_ok=True)
    with OUT.open("w", encoding="utf-8") as f:
        f.write("cod_dpto,dpto,cod_mpio,nom_mpio,tipo,lon,lat\n")
        for r in rows:
            f.write(",".join([
                csv_cell(r.get("cod_dpto", "")),
                csv_cell(r.get("dpto", "")),
                csv_cell(r.get("cod_mpio", "")),
                csv_cell(r.get("nom_mpio", "")),
                csv_cell(r.get("tipo_municipio", "")),
                num(r.get("longitud", "")),
                num(r.get("latitud", "")),
            ]) + "\n")

    deptos = len({r.get("cod_dpto") for r in rows})
    print(f"OK -> {OUT.relative_to(ROOT)}  ({len(rows)} entidades, {deptos} departamentos)")


if __name__ == "__main__":
    main()
