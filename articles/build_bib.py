#!/usr/bin/env python3
"""
Socium — Base bibliográfica universal e incremental.

Escanea articles/pdfs/, extrae/enriquece metadatos y:
  - AÑADE a references.bib SOLO las entradas nuevas (nunca modifica las existentes),
  - regenera catalog.md (tabla navegable de toda la colección).

Estrategia de identificación (de más a menos fiable):
  1) DOI encontrado dentro del PDF        -> Crossref por DOI (exacto)
  2) arXiv-id (nombre o texto)            -> API de arXiv (exacto)
  3) Título (metadato o 1ª página)        -> Crossref por título CON verificación
  4) Si nada es fiable                    -> entrada @misc mínima (revisar a mano)

Clave BibTeX = nombre de archivo (sin ext.), mapeo 1:1 con el PDF (campo `file`).

Uso:
    python3 articles/build_bib.py            # online (Crossref/arXiv)
    python3 articles/build_bib.py --offline  # solo metadatos locales
    python3 articles/build_bib.py --dry-run  # no escribe, solo reporta

Sin dependencias externas: poppler (pdfinfo/pdftotext) + stdlib (urllib).
"""
from __future__ import annotations
import argparse, json, re, subprocess, sys, time, urllib.parse, urllib.request
from pathlib import Path

HERE = Path(__file__).resolve().parent
PDF_DIR = HERE / "pdfs"
BIB = HERE / "references.bib"
CATALOG = HERE / "catalog.md"
MAILTO = "sr260035@gmail.com"
UA = f"Socium-bib/0.2 (mailto:{MAILTO})"

DOI_RE = re.compile(r"10\.\d{4,9}/[-._;()/:A-Za-z0-9]+", re.I)
ARXIV_RE = re.compile(r"(\d{4}\.\d{4,5})(v\d+)?")
STOP = set("a an the of and or for on in to with from by as at is study model models "
           "approach analysis using based effect effects new".split())


# ---------- utilidades ----------
def run(cmd):
    try:
        return subprocess.run(cmd, capture_output=True, text=True, timeout=60).stdout
    except Exception:
        return ""

def slugify_key(stem):
    return re.sub(r"[^A-Za-z0-9]+", "_", stem).strip("_") or "ref"

def existing_keys(text):
    return set(re.findall(r"@\w+\s*\{\s*([^,\s]+)\s*,", text))

def _fetch(url, accept=None, tries=3):
    h = {"User-Agent": UA}
    if accept:
        h["Accept"] = accept
    last = None
    for i in range(tries):
        try:
            with urllib.request.urlopen(urllib.request.Request(url, headers=h), timeout=35) as r:
                return r.read().decode("utf-8", "replace")
        except Exception as e:
            last = e
            time.sleep(1.5 * (i + 1))
    raise last

def http_json(url):
    return json.loads(_fetch(url, accept="application/json"))

def http_text(url):
    return _fetch(url)

def tokens(s):
    return {w for w in re.findall(r"[a-z]+", s.lower()) if len(w) > 2 and w not in STOP}

def is_garbage_title(g, stem):
    if not g or g == stem:
        return True
    if re.search(r"\.(dvi|tmp|tex)\b|^PII:|^b\d+\b|microsoft word|untitled|^figure |^table |^poster", g, re.I):
        return True
    return len(tokens(g)) < 2


# ---------- extracción local ----------
def pdf_text(pdf, pages=3):
    return run(["pdftotext", "-f", "1", "-l", str(pages), str(pdf), "-"])

def find_doi(text):
    for m in DOI_RE.finditer(text):
        return m.group(0).rstrip(".,;)")
    return None

def find_arxiv(stem, text):
    m = ARXIV_RE.fullmatch(stem) or ARXIV_RE.match(stem)
    if m:
        return m.group(1)
    m = re.search(r"arXiv:\s*" + ARXIV_RE.pattern, text)
    return m.group(1) if m else None

def title_guess(pdf):
    info = run(["pdfinfo", str(pdf)])
    m = re.search(r"^Title:\s*(.+)$", info, re.M)
    if m:
        t = m.group(1).strip()
        if len(t) > 8 and ".tmp" not in t.lower() and "microsoft word" not in t.lower():
            return t
    txt = run(["pdftotext", "-f", "1", "-l", "1", str(pdf), "-"])
    lines = [l.strip() for l in txt.splitlines() if len(l.strip()) > 12]
    lines = [l for l in lines if not re.match(r"^(vol\.|doi|http|©|arxiv|received|copyright|figure|table|poster|abstract)", l, re.I)]
    return max(lines[:8], key=len) if lines else pdf.stem


# ---------- online ----------
def crossref_by_doi(doi):
    try:
        return http_json(f"https://api.crossref.org/works/{urllib.parse.quote(doi)}?mailto={MAILTO}")["message"]
    except Exception:
        return None

def crossref_by_title(title):
    url = ("https://api.crossref.org/works?rows=1&mailto=" + MAILTO +
           "&query.bibliographic=" + urllib.parse.quote(title))
    items = http_json(url).get("message", {}).get("items", [])
    return items[0] if items else None

def arxiv_by_id(aid):
    xml = http_text(f"http://export.arxiv.org/api/query?id_list={aid}&max_results=1")
    t = re.search(r"<entry>.*?<title>(.*?)</title>", xml, re.S)
    if not t:
        return None
    authors = re.findall(r"<author>\s*<name>(.*?)</name>", xml, re.S)
    y = re.search(r"<published>(\d{4})", xml)
    doi = re.search(r"<arxiv:doi[^>]*>(.*?)</arxiv:doi>", xml)
    return {"title": re.sub(r"\s+", " ", t.group(1)).strip(), "authors": authors,
            "year": y.group(1) if y else "", "id": aid, "doi": doi.group(1) if doi else None}


# ---------- BibTeX ----------
CR_TYPE = {"journal-article": "article", "proceedings-article": "inproceedings",
           "book": "book", "book-chapter": "incollection", "monograph": "book",
           "edited-book": "book", "reference-book": "book",
           "posted-content": "misc", "report": "techreport"}

def cr_authors(item):
    out = []
    for a in item.get("author", []) or []:
        fam = a.get("family", "")
        out.append(f"{fam}, {a.get('given','')}".strip(", ") if fam else a.get("name", ""))
    return " and ".join(x for x in out if x)

def cr_year(item):
    for k in ("published-print", "published-online", "issued", "created"):
        dp = item.get(k, {}).get("date-parts", [[None]])
        if dp and dp[0] and dp[0][0]:
            return str(dp[0][0])
    return ""

def cr_title(item):
    return " ".join(item.get("title", []) or []) or ""

def fmt(etype, key, fields):
    body = ",\n".join(f"  {k:<12}= {{{v}}}" for k, v in fields.items() if v)
    return f"@{etype}{{{key},\n{body}\n}}\n"

def entry_crossref(key, item, rel):
    etype = CR_TYPE.get(item.get("type", ""), "misc")
    cont = " ".join(item.get("container-title", []) or [])
    f = {"author": cr_authors(item), "title": cr_title(item), "year": cr_year(item),
         "journal": cont if etype == "article" else "",
         "booktitle": cont if etype in ("inproceedings", "incollection") else "",
         "publisher": item.get("publisher", "") if etype in ("book", "incollection") else "",
         "volume": item.get("volume", ""), "number": item.get("issue", ""),
         "pages": item.get("page", ""), "doi": item.get("DOI", ""), "file": rel}
    return etype, f

def entry_arxiv(key, meta, rel):
    return "article", {"author": " and ".join(meta["authors"]), "title": meta["title"],
                       "year": meta["year"], "eprint": meta["id"], "archiveprefix": "arXiv",
                       "doi": meta.get("doi") or "", "file": rel}


# ---------- principal ----------
def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--offline", action="store_true")
    ap.add_argument("--dry-run", action="store_true")
    args = ap.parse_args()
    if not PDF_DIR.is_dir():
        sys.exit(f"No existe {PDF_DIR}")

    have = existing_keys(BIB.read_text(encoding="utf-8")) if BIB.exists() else set()
    pdfs = sorted(PDF_DIR.glob("*.pdf"))
    new_entries, rows, added = [], [], 0

    for pdf in pdfs:
        key, rel = slugify_key(pdf.stem), f"pdfs/{pdf.name}"
        if key in have:
            rows.append((key, "(ya en .bib)", "", "", "", "—", rel)); continue

        etype = fields = None
        title = authors = year = doi = ""
        src = "local"
        text = pdf_text(pdf) if not args.offline else ""

        if not args.offline:
            try:
                doi_hit = find_doi(text)
                aid = find_arxiv(pdf.stem, text)
                if doi_hit:
                    item = crossref_by_doi(doi_hit)
                    if item:
                        etype, fields = entry_crossref(key, item, rel); src = "doi"
                if fields is None and aid:
                    meta = arxiv_by_id(aid)
                    if meta:
                        etype, fields = entry_arxiv(key, meta, rel); src = "arxiv"
                if fields is None:                          # título con verificación
                    g = title_guess(pdf)
                    if not is_garbage_title(g, pdf.stem):
                        item = crossref_by_title(g)
                        shared = tokens(g) & tokens(cr_title(item)) if item else set()
                        if item and len(shared) >= 3 and len(shared) >= 0.5 * len(tokens(g)):
                            etype, fields = entry_crossref(key, item, rel); src = "title"
                time.sleep(0.34)
            except Exception as e:
                print(f"  ! {pdf.name}: online falló ({e})", file=sys.stderr)

        if fields is None:                                  # fallback mínimo
            etype, fields, src = "misc", {"title": title_guess(pdf),
                                          "note": "REVISAR: metadatos sin verificar",
                                          "file": rel}, "sin-id"

        new_entries.append(fmt(etype, key, fields))
        title = fields.get("title", ""); authors = fields.get("author", "")
        year = fields.get("year", ""); doi = fields.get("doi", "")
        rows.append((key, title[:65], authors[:40], year, doi, src, rel))
        added += 1
        flag = "" if src in ("doi", "arxiv") else ("  [verif:título]" if src == "title" else "  [SIN ID — revisar]")
        print(f"  + [{src:6}] {key[:42]:42} :: {title[:50]}{flag}")

    if new_entries and not args.dry_run:
        with BIB.open("a", encoding="utf-8") as fh:
            fh.write(f"\n% --- build_bib.py {time.strftime('%Y-%m-%d %H:%M')} ---\n")
            fh.write("\n".join(new_entries))

    if not args.dry_run:
        out = ["# Catálogo bibliográfico — Socium\n",
               f"_Generado por `build_bib.py` el {time.strftime('%Y-%m-%d %H:%M')} — {len(pdfs)} PDFs._\n",
               "Columna **Fuente**: `doi`/`arxiv` = fiable · `title` = verificado por título · "
               "`sin-id` = revisar a mano.\n",
               "| Clave | Título | Autores | Año | Fuente | DOI | Archivo |",
               "|-------|--------|---------|-----|--------|-----|---------|"]
        for key, t, a, y, d, src, rel in rows:
            dm = f"[{d}](https://doi.org/{d})" if d else ""
            out.append(f"| `{key}` | {t} | {a} | {y} | {src} | {dm} | `{rel}` |")
        CATALOG.write_text("\n".join(out) + "\n", encoding="utf-8")

    print(f"\nResumen: {len(pdfs)} PDFs · {added} nuevas · {len(pdfs)-added} ya estaban.")
    if args.dry_run:
        print("(dry-run: no se escribió nada)")


if __name__ == "__main__":
    main()
