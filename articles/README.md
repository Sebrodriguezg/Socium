# Bibliografía (`articles/`)

Toda la base de literatura del proyecto vive aquí y **se versiona en Git**
(los PDFs vía **Git LFS**, configurado en `.gitattributes`).

## Estructura

- `references.bib` — base bibliográfica central (BibTeX). Una entrada por trabajo.
- `pdfs/` — los archivos PDF (gestionados por Git LFS).
- `notes/` — una ficha en Markdown por artículo (resumen, ideas clave, cómo se
  conecta con el modelo). Nómbralas igual que la clave BibTeX, p.ej. `epstein1996growing.md`.
- `by-topic/` — organización temática (atajos/listas por subárea):
  - `econophysics/` — econofísica, distribuciones de riqueza, mercados.
  - `agent-based-models/` — metodología ABM, frameworks, ODD.
  - `social-networks/` — redes sociales, formación de lazos, homofilia.
  - `opinion-dynamics/` — dinámica de opiniones, polarización.
  - `inequality-wealth/` — desigualdad, movilidad, Gini.
  - `education-policy/` — educación y capital humano.
  - `labor-markets/` — mercados laborales, empleo, matching.
  - `demography-mating/` — demografía, emparejamiento, hogares.
  - `health-policy/` — salud pública y políticas sanitarias.
  - `hpc-methods/` — paralelización, escalabilidad, métodos numéricos.
  - `calibration-validation/` — calibración y validación empírica.

## Antes de hacer commit de PDFs

Instala Git LFS una sola vez:

```bash
git lfs install
git lfs track "articles/pdfs/**/*.pdf"   # ya está en .gitattributes
```
