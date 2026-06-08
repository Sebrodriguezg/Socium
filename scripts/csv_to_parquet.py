#!/usr/bin/env python3
"""
Convierte el flujo CSV de la muestra de agentes (stdin) a Parquet (streaming, memoria acotada).
Pensado para tubería: el motor escribe la muestra a stdout y aquí se va escribiendo Parquet
sin guardar el CSV gigante en disco.

    socium_simular ... --out /dev/null --out-muestra /dev/stdout 2>/dev/null \
        | python3 scripts/csv_to_parquet.py web/data/agentes.parquet
"""
import sys
import pyarrow as pa
import pyarrow.csv as pacsv
import pyarrow.parquet as pq

out = sys.argv[1] if len(sys.argv) > 1 else "agentes.parquet"
esc = sys.argv[2] if len(sys.argv) > 2 else None   # etiqueta de escenario (columna constante)
reader = pacsv.open_csv(sys.stdin.buffer, read_options=pacsv.ReadOptions(block_size=16 << 20))
writer = None
n = 0
for batch in reader:                      # cada RecordBatch se escribe y se libera
    t = pa.Table.from_batches([batch])
    if esc is not None:
        t = t.append_column("escenario", pa.array([esc] * t.num_rows, pa.string()))
    if writer is None:
        writer = pq.ParquetWriter(out, t.schema, compression="zstd")
    writer.write_table(t)
    n += t.num_rows
if writer:
    writer.close()
print(f"parquet: {n} filas -> {out}", file=sys.stderr)
