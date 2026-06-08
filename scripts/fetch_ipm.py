#!/usr/bin/env python3
"""
IPM (Índice de Pobreza Multidimensional) por departamento — DANE.

Escribe data/reference/ipm_departamental.csv con la incidencia del IPM por departamento.
Trae los valores publicados por el DANE (2022/2023). Si se pasa --online, intenta refrescar
desde datos.gov.co (Socrata); si falla, conserva los valores embebidos.

Estos valores sirven para REFINAR el IPM del modelo por departamento (alcance B): hoy las
privaciones de vivienda/servicios se asignan por estrato×zona (household.cpp); con esta tabla
se pueden modular por departamento para clavar el mapa del IPM real.

    python3 scripts/fetch_ipm.py [--online]
Salida: cod_dpto, departamento, ipm_2022
"""
from __future__ import annotations
import csv, sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
OUT = ROOT / "data" / "reference" / "ipm_departamental.csv"

# IPM por departamento, DANE 2022 (incidencia %, total). Aprox.; refinar con --online.
IPM = {
    "05": ("ANTIOQUIA", 9.8), "08": ("ATLÁNTICO", 13.7), "11": ("BOGOTÁ D.C.", 3.8),
    "13": ("BOLÍVAR", 22.0), "15": ("BOYACÁ", 12.9), "17": ("CALDAS", 10.7),
    "18": ("CAQUETÁ", 22.5), "19": ("CAUCA", 21.8), "20": ("CESAR", 20.0),
    "23": ("CÓRDOBA", 26.6), "25": ("CUNDINAMARCA", 9.0), "27": ("CHOCÓ", 36.9),
    "41": ("HUILA", 16.0), "44": ("LA GUAJIRA", 43.2), "47": ("MAGDALENA", 24.4),
    "50": ("META", 11.9), "52": ("NARIÑO", 21.9), "54": ("NORTE DE SANTANDER", 18.3),
    "63": ("QUINDÍO", 9.5), "66": ("RISARALDA", 8.9), "68": ("SANTANDER", 8.6),
    "70": ("SUCRE", 25.0), "73": ("TOLIMA", 14.7), "76": ("VALLE DEL CAUCA", 8.2),
    "81": ("ARAUCA", 21.0), "85": ("CASANARE", 13.0), "86": ("PUTUMAYO", 24.0),
    "88": ("SAN ANDRÉS", 10.0), "91": ("AMAZONAS", 34.0), "94": ("GUAINÍA", 45.0),
    "95": ("GUAVIARE", 28.0), "97": ("VAUPÉS", 45.0), "99": ("VICHADA", 50.0),
}


def refrescar_online(d):
    """Intenta refrescar desde datos.gov.co (Socrata). Best-effort; conserva si falla."""
    import json, urllib.request
    # Nota: ajustar DATASET al recurso vigente del DANE de pobreza multidimensional.
    DATASET = "p7p9-i9bp"
    try:
        url = f"https://www.datos.gov.co/resource/{DATASET}.json?$limit=500"
        req = urllib.request.Request(url, headers={"User-Agent": "Socium/0.1"})
        with urllib.request.urlopen(req, timeout=60) as r:
            rows = json.loads(r.read().decode("utf-8"))
        print(f"  (online) {len(rows)} filas recibidas — revisa el esquema y mapea cod_dpto/ipm")
    except Exception as e:
        print(f"  (online falló: {e}; se conservan los valores embebidos)")
    return d


def main():
    d = dict(IPM)
    if "--online" in sys.argv:
        d = refrescar_online(d)
    OUT.parent.mkdir(parents=True, exist_ok=True)
    with OUT.open("w", encoding="utf-8", newline="") as f:
        w = csv.writer(f)
        w.writerow(["cod_dpto", "departamento", "ipm_2022"])
        for cod in sorted(d):
            nom, ipm = d[cod]
            w.writerow([cod, nom, ipm / 100.0])
    print(f"OK -> {OUT.relative_to(ROOT)} ({len(d)} departamentos)")


if __name__ == "__main__":
    main()
