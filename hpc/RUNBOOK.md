# Runbook — correr Socium en el clúster

Paso a paso para clonar, construir y correr el modelo a escala (hasta 50M agentes = 1:1
con Colombia) y regenerar los datos de la web. **Las bases de datos vienen con el repo**
(`data/reference/*.csv`), así que NO hay que descargarlas por separado.

## 0. Requisitos
El nodo ya tiene: g++ 11 (C++20+OpenMP), git, miniconda. Sin sudo. Usaremos conda para
lo que falte (cmake, pyyaml, etc.).

## 1. Clonar el repositorio
```bash
git clone https://github.com/Sebrodriguezg/Socium.git
cd Socium
git checkout develop
```
(Si prefieres SSH y tienes la llave en el nodo: `git clone git@github.com:Sebrodriguezg/Socium.git`.)

Los datos ya están: `ls data/reference/` debe mostrar `divipola.csv`,
`poblacion_municipal.csv`, `municipios.csv`, `conflicto_municipal.csv`,
`pib_departamental.csv`, `parametros.yaml`, etc.

## 2. Entorno (una sola vez)
```bash
conda env create -f hpc/environment.yml   # trae cmake, ninja, pyyaml, numpy...
conda activate socium
```
(Si ya existe: `conda env update -f hpc/environment.yml --prune` y `conda activate socium`.)

## 3. Correr todo con un comando
```bash
bash scripts/correr_cluster.sh 50000000 128     # 50M agentes, 128 hilos
```
Esto: compila el núcleo (CMake, `-march=native`), genera las palancas de los 3 escenarios,
corre la simulación 2026→2030 para cada uno (con muestra, perfiles, series, mapa) y
empaqueta `web/data/*.json`. Memoria a 50M ≈ 6-10 GB (sobra en 251 GB); tiempo ≈ minutos
por escenario en 128 hilos.

Para una prueba rápida primero: `bash scripts/correr_cluster.sh 5000000 64`.

## 4. (Opcional) refrescar los datos desde las fuentes
Solo si quieres re-descargar de DANE/datos.gov.co (requiere internet en el nodo):
```bash
python3 scripts/fetch_divipola.py
python3 scripts/fetch_poblacion_total.py 2026
python3 scripts/fetch_poblacion_municipal.py 2024
python3 scripts/fetch_conflicto.py 2022
python3 scripts/fetch_pib_departamental.py 2023
```

## 5. Validar y comparar
```bash
python3 python/socium/viz/validar.py             # validación espacial vs DANE
python3 python/socium/viz/validar_historico.py   # fuera de muestra 2018-2023
python3 python/socium/viz/stylized_facts.py      # hechos estilizados
python3 python/socium/viz/comparar_escenarios.py # comparación + figuras
python3 scripts/ensemble.py --n 8000000 --reps 64 --jobs 8 --threads 16  # incertidumbre
```

## 6. Ver / publicar la web
- **Ver local en el nodo** (y abrir por túnel SSH `-L 8000:localhost:8000`):
  ```bash
  cd web && python3 -m http.server 8000   # http://localhost:8000
  ```
- **Publicar** (actualiza GitHub Pages con los datos a 50M):
  ```bash
  git add web && git commit -m "datos a 50M (cluster)"
  git push origin develop
  git subtree push --prefix web origin gh-pages
  ```
  > Nota: `web/data/*.json` a 50M pesa más (la muestra crece). Si supera ~50 MB conviene
  > subir el `--muestra-paso` (menos agentes en la muestra) o usar Git LFS para esos JSON.

## 7. Corridas grandes sin que se corten
Como no hay scheduler, usa `tmux` o `nohup` para que sobreviva al cierre de sesión:
```bash
tmux new -s socium
bash scripts/correr_cluster.sh 50000000 128
#  Ctrl-b d  para desconectar;  tmux attach -t socium  para volver
```
