#!/usr/bin/env python3
"""
Servidor estático con soporte de Range (rangos de bytes) para servir web/ en local.
Necesario para que DuckDB-WASM lea los Parquet por partes (sin bajar el archivo entero).

    python3 web/serve.py [puerto]      # por defecto 8000  ->  http://localhost:8000
"""
import http.server, os, re, sys

PORT = int(sys.argv[1]) if len(sys.argv) > 1 else 8000
WEBDIR = os.path.dirname(os.path.abspath(__file__))


class H(http.server.SimpleHTTPRequestHandler):
    def __init__(self, *a, **k):
        super().__init__(*a, directory=WEBDIR, **k)

    def end_headers(self):
        self.send_header("Accept-Ranges", "bytes")
        super().end_headers()

    def do_GET(self):
        rng = self.headers.get("Range")
        path = self.translate_path(self.path)
        if rng and os.path.isfile(path):
            m = re.match(r"bytes=(\d+)-(\d*)", rng)
            if m:
                size = os.path.getsize(path)
                start = int(m.group(1))
                end = int(m.group(2)) if m.group(2) else size - 1
                end = min(end, size - 1)
                length = end - start + 1
                self.send_response(206)
                self.send_header("Content-Type", self.guess_type(path))
                self.send_header("Content-Range", f"bytes {start}-{end}/{size}")
                self.send_header("Content-Length", str(length))
                self.end_headers()
                with open(path, "rb") as f:
                    f.seek(start)
                    self.wfile.write(f.read(length))
                return
        super().do_GET()


http.server.ThreadingHTTPServer.allow_reuse_address = True
with http.server.ThreadingHTTPServer(("", PORT), H) as s:
    print(f"Socium web en http://localhost:{PORT}  (soporte de rangos para DuckDB) — Ctrl-C para parar")
    s.serve_forever()
