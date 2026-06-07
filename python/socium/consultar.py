#!/usr/bin/env python3
"""
Consulta pública: "¿cómo sería mi vida, en promedio?"
Dado un perfil (edad, sexo, departamento, educación), devuelve los resultados promedio
de personas con ese perfil, año a año (2026->2030), bajo cada escenario de gobierno.

Lee experiments/runs/perfiles_<escenario>.csv (generados con socium_simular --out-perfiles).

Ejemplos:
  python3 python/socium/consultar.py --edad 30 --sexo M --depto "Chocó" --educacion bajo
  python3 python/socium/consultar.py --edad 22 --sexo F --depto "Bogotá" --educacion alto --anio 2030
"""
from __future__ import annotations
import argparse, csv, unicodedata
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
RUNS = ROOT / "experiments" / "runs"
DIVIPOLA = ROOT / "data" / "reference" / "divipola.csv"
ESCENARIOS = ["baseline", "cepeda", "de_la_espriella"]
NOMBRE = {"baseline": "Baseline", "cepeda": "Cepeda", "de_la_espriella": "De la Espriella"}


def norm(s): return unicodedata.normalize("NFKD", s).encode("ascii","ignore").decode().upper().strip()


def edad_label(e):
    e = int(e)
    for hi, lab in [(15,"0-14"),(25,"15-24"),(35,"25-34"),(45,"35-44"),(55,"45-54"),(65,"55-64")]:
        if e < hi: return lab
    return "65+"


def educ_label(s):
    s = s.lower().strip()
    if s in ("bajo","medio","alto"): return s
    # acepta años de escolaridad o nivel
    try:
        a = int(s)
        return "bajo" if a <= 5 else "medio" if a <= 11 else "alto"
    except ValueError:
        if s in ("ninguno","primaria"): return "bajo"
        if s in ("secundaria","media","bachiller"): return "medio"
        return "alto"


def cod_depto(nombre):
    d = {}
    for r in csv.DictReader(open(DIVIPOLA)):
        d[norm(r["dpto"])] = r["cod_dpto"]
    q = norm(nombre)
    if q in d: return d[q]
    for k, v in d.items():
        if q in k or k in q: return v
    return None


def sexo_norm(s):
    s = s.lower()[:1]
    return "Mujer" if s in ("m","f") and s != "h" else ("Hombre" if s == "h" else ("Mujer" if s=="f" else "Hombre"))


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--edad", required=True)
    ap.add_argument("--sexo", required=True, help="H/M (Hombre/Mujer)")
    ap.add_argument("--depto", required=True, help="nombre del departamento")
    ap.add_argument("--educacion", required=True, help="bajo/medio/alto o años o nivel")
    ap.add_argument("--situacion", default="empleado_formal",
                    help="empleado_publico/empleado_formal/cuenta_propia/informal/desempleado/inactivo/estudiante")
    ap.add_argument("--anio", default=None)
    a = ap.parse_args()

    eb, ub = edad_label(a.edad), educ_label(a.educacion)
    sx = "Hombre" if a.sexo.lower().startswith("h") else "Mujer"
    cod = cod_depto(a.depto)
    if not cod:
        raise SystemExit(f"Departamento no reconocido: {a.depto}")

    print(f"\nPerfil: {a.edad} años, {sx}, {a.depto} (educación {ub}, {a.situacion})")
    print("="*78)

    def fila(esc):
        out = {}
        f = RUNS / f"perfiles_{esc}.csv"
        if not f.exists(): return out
        for r in csv.DictReader(open(f)):
            if (r["cod_dpto"]==cod and r["sexo"]==sx and r["edad"]==eb and r["educacion"]==ub
                    and r["situacion"]==a.situacion and (a.anio is None or r["anio"]==str(a.anio))):
                out[r["anio"]] = r
        return out

    datos = {e: fila(e) for e in ESCENARIOS}
    anios = sorted(datos["baseline"].keys())
    if not anios:
        raise SystemExit("No hay datos para ese perfil (muestra muy pequeña). Prueba un perfil más común.")

    METR = [("ingreso_pc","ingreso pc (COP/mes)",lambda v:f"${int(float(v)):,}"),
            ("p_ocupado","prob. de estar ocupado",lambda v:f"{float(v)*100:.0f}%"),
            ("p_pobreza","prob. de ser pobre",lambda v:f"{float(v)*100:.0f}%"),
            ("p_enfermo","prob. de enfermar",lambda v:f"{float(v)*100:.0f}%"),
            ("p_delincuencia","prob. delincuencia",lambda v:f"{float(v)*100:.1f}%"),
            ("satisfaccion","satisfacción (0-10)",lambda v:f"{float(v):.1f}")]

    for anio in anios:
        print(f"\n— Año {anio} —")
        print(f"  {'métrica':24}" + "".join(f"{NOMBRE[e]:>16}" for e in ESCENARIOS))
        for key, lab, fmt in METR:
            row = f"  {lab:24}"
            for e in ESCENARIOS:
                row += f"{(fmt(datos[e][anio][key]) if anio in datos[e] else '-'):>16}"
            print(row)
    print("\n(Promedios de agentes con tu mismo perfil. Modelo Socium, calibrado a DANE 2023.)")


if __name__ == "__main__":
    main()
