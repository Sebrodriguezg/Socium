#!/usr/bin/env python3
"""
Descarga el anexo oficial de DANE (proyecciones de población municipal por área,
2020-2035, actualización post-COVID) y extrae la población TOTAL por municipio para
un año dado (por defecto 2026, el "Colombia de hoy"), con desglose urbano/rural.

Salida: data/reference/poblacion_municipal.csv
Columnas: cod_mpio, poblacion_total, poblacion_cabecera, poblacion_rural, pct_urbano

Parsea el .xlsx sin dependencias (zipfile + XML).

    python3 scripts/fetch_poblacion_total.py [anio]   # por defecto 2026
"""
from __future__ import annotations
import sys, zipfile, urllib.request
import xml.etree.ElementTree as ET
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
OUT = ROOT / "data" / "reference" / "poblacion_municipal.csv"
URL = ("https://www.dane.gov.co/files/censo2018/proyecciones-de-poblacion/"
       "Municipal/DCD-area-proypoblacion-Mun-2020-2035-ActPostCOVID-19.xlsx")
NS = "{http://schemas.openxmlformats.org/spreadsheetml/2006/main}"
ANIO = sys.argv[1] if len(sys.argv) > 1 else "2026"


def col_letter(ref: str) -> str:
    return "".join(ch for ch in ref if ch.isalpha())


def parse_rows(xlsx: Path):
    z = zipfile.ZipFile(xlsx)
    ss = ["".join(t.text or "" for t in n.iter(NS + "t"))
          for n in ET.fromstring(z.read("xl/sharedStrings.xml"))]
    root = ET.fromstring(z.read("xl/worksheets/sheet1.xml"))
    for row in root.find(NS + "sheetData").findall(NS + "row"):
        cells = {}
        for c in row.findall(NS + "c"):
            v = c.find(NS + "v")
            if v is None:
                continue
            cells[col_letter(c.get("r"))] = ss[int(v.text)] if c.get("t") == "s" else v.text
        if cells:
            yield cells


def main():
    tmp = Path("/tmp/dane_proj.xlsx")
    if not tmp.exists():
        print("descargando anexo DANE...")
        req = urllib.request.Request(URL, headers={"User-Agent": "Socium/0.1"})
        with urllib.request.urlopen(req, timeout=120) as r:
            tmp.write_bytes(r.read())

    # columnas: C=cod_mpio, E=año, F=área, G=población
    tot, cab, rur = {}, {}, {}
    for c in parse_rows(tmp):
        if c.get("E") != ANIO:
            continue
        cod, area, pob = c.get("C", ""), c.get("F", ""), c.get("G", "")
        if not cod or not pob:
            continue
        try:
            p = int(float(pob))
        except ValueError:
            continue
        if area == "Total":
            tot[cod] = p
        elif area.startswith("Cabecera"):
            cab[cod] = p
        elif area.startswith("Centros"):
            rur[cod] = p

    OUT.parent.mkdir(parents=True, exist_ok=True)
    with OUT.open("w", encoding="utf-8") as f:
        f.write("cod_mpio,poblacion_total,poblacion_cabecera,poblacion_rural,pct_urbano\n")
        for cod in sorted(tot):
            t = tot[cod]
            cb = cab.get(cod, 0)
            rr = rur.get(cod, 0)
            pct = round(100.0 * cb / t, 2) if t else 0.0
            f.write(f"{cod},{t},{cb},{rr},{pct}\n")

    total_nac = sum(tot.values())
    print(f"OK -> {OUT.relative_to(ROOT)}  (año {ANIO}, {len(tot)} municipios, "
          f"población nacional {total_nac:,})")


if __name__ == "__main__":
    main()
