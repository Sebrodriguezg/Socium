# Web pública — "¿Cómo sería tu vida?"

Página **estática** (HTML + JS, sin servidor) que consulta los perfiles del modelo en el
navegador: una persona ingresa edad, sexo, departamento y nivel educativo, y ve cómo le
iría —en promedio, año a año— bajo cada escenario de gobierno (Cepeda vs De la Espriella
vs continuidad).

## Archivos
- `index.html` — la aplicación (formulario + lógica de consulta).
- `data/perfiles.json` — perfiles agregados (depto×sexo×edad×educación × año × escenario).
- `data/departamentos.json` — catálogo de departamentos para el menú.

## Regenerar los datos
```bash
# 1) correr los 3 escenarios exportando perfiles
for s in baseline cepeda de_la_espriella; do
  ./build/socium_simular --n 8000000 --years 4 \
      --escenario data/reference/escenario_$s.flat \
      --out /dev/null --out-perfiles experiments/runs/perfiles_$s.csv
done
# 2) empaquetar a JSON para la web
python3 scripts/build_web_data.py
```

## Probar localmente
```bash
cd web && python3 -m http.server 8000   # abrir http://localhost:8000
```

## Publicar (GitHub Pages, gratis)
Como Pages sirve desde la raíz o `/docs`, la forma simple es una rama `gh-pages`:
```bash
git subtree push --prefix web origin gh-pages
```
y en GitHub: Settings → Pages → Source: rama `gh-pages`. Quedará en
`https://sebrodriguezg.github.io/Socium/`.

> Nota: las cifras son **escenarios de simulación** calibrados a DANE 2023, con
> tratamiento neutral de ambos candidatos; no son predicciones.
