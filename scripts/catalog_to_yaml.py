#!/usr/bin/env python3
"""
Convierte el catálogo de métricas (docs/model-spec/metrics-catalog.md) en un
esquema legible por el código: docs/model-spec/metrics.yaml.

Parsea las tablas Markdown de cada dominio (encabezados "## N. Título") y emite
una entrada por métrica con: id, nombre, definición/unidad, niveles, roles y fuente.

Reproducible: re-ejecutar tras editar el catálogo mantiene el YAML sincronizado.
Sin dependencias externas (emite YAML a mano).

    python3 scripts/catalog_to_yaml.py
"""
from __future__ import annotations
import re, sys, time, unicodedata
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
SRC = ROOT / "docs" / "model-spec" / "metrics-catalog.md"
OUT = ROOT / "docs" / "model-spec" / "metrics.yaml"

LEVELS = {"Ag", "Hg", "Mun", "Dep", "Nac"}
ROLES = {"E", "A", "P", "C"}


def slug(s: str) -> str:
    s = re.sub(r"\*\*|\*|`", "", s)                       # quita markdown
    s = unicodedata.normalize("NFKD", s).encode("ascii", "ignore").decode()
    s = re.sub(r"[^A-Za-z0-9]+", "_", s).strip("_").lower()
    return s or "metric"


def clean(s: str) -> str:
    return re.sub(r"\*\*|\*", "", s).strip()


def split_tokens(cell: str, allowed: set[str]) -> list[str]:
    found = []
    for tok in re.split(r"[\s,/]+", cell):
        tok = tok.strip()
        if tok in allowed and tok not in found:
            found.append(tok)
    return found


def yaml_str(s: str) -> str:
    return '"' + s.replace("\\", "\\\\").replace('"', '\\"') + '"'


def main():
    if not SRC.exists():
        sys.exit(f"No existe {SRC}")
    lines = SRC.read_text(encoding="utf-8").splitlines()

    domains, cur, seen_header, total = [], None, False, 0
    for ln in lines:
        m = re.match(r"^##\s+(\d+)\.\s+(.*)$", ln)
        if m:
            cur = {"number": int(m.group(1)), "title": clean(m.group(2)),
                   "id": slug(m.group(2)), "metrics": []}
            domains.append(cur)
            seen_header = False
            continue
        if cur is None or not ln.lstrip().startswith("|"):
            continue
        cells = [c.strip() for c in ln.strip().strip("|").split("|")]
        if len(cells) < 5:
            continue
        if set(cells[0]) <= set("-: "):                   # separador |---|
            continue
        if cells[0].lower().startswith("métrica") or cells[0].lower() == "metrica":
            seen_header = True
            continue
        if not seen_header:
            continue
        name = clean(cells[0])
        if not name:
            continue
        cur["metrics"].append({
            "id": slug(name),
            "name": name,
            "definition": clean(cells[1]),
            "levels": split_tokens(cells[2], LEVELS),
            "roles": split_tokens(cells[3], ROLES),
            "source": clean(cells[4]),
        })
        total += 1

    domains = [d for d in domains if d["metrics"]]

    out = []
    out.append("# Esquema de métricas de Socium · Colombia (GENERADO)")
    out.append(f"# Fuente: docs/model-spec/metrics-catalog.md")
    out.append(f"# Generado por scripts/catalog_to_yaml.py el {time.strftime('%Y-%m-%d %H:%M')}")
    out.append("# NO editar a mano: editar el catálogo .md y re-ejecutar el conversor.")
    out.append("#")
    out.append("# niveles: Ag=individuo Hg=hogar Mun=municipio Dep=departamento Nac=nación")
    out.append("# roles:   E=estado-agente A=agregado-emergente P=palanca-política C=calibración")
    out.append("meta:")
    out.append(f"  source: docs/model-spec/metrics-catalog.md")
    out.append(f"  domains: {len(domains)}")
    out.append(f"  metrics: {total}")
    out.append("domains:")
    for d in domains:
        out.append(f"  - id: {d['id']}")
        out.append(f"    number: {d['number']}")
        out.append(f"    title: {yaml_str(d['title'])}")
        out.append(f"    metrics:")
        for mt in d["metrics"]:
            out.append(f"      - id: {mt['id']}")
            out.append(f"        name: {yaml_str(mt['name'])}")
            out.append(f"        definition: {yaml_str(mt['definition'])}")
            out.append(f"        levels: [{', '.join(mt['levels'])}]")
            out.append(f"        roles: [{', '.join(mt['roles'])}]")
            out.append(f"        source: {yaml_str(mt['source'])}")
    OUT.write_text("\n".join(out) + "\n", encoding="utf-8")
    print(f"OK -> {OUT.relative_to(ROOT)}  ({len(domains)} dominios, {total} métricas)")


if __name__ == "__main__":
    main()
