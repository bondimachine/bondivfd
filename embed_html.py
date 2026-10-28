#!/usr/bin/env python3
"""Convierte examples/BondiWeb/index.html en index_html.h para que el ESP32
lo sirva desde la flash. Correr despues de editar index.html."""
import os

DIR = os.path.join(os.path.dirname(os.path.abspath(__file__)), "examples", "BondiWeb")
html = open(os.path.join(DIR, "index.html"), encoding="utf-8").read()
html = "\n".join(line.strip() for line in html.splitlines() if line.strip())
assert ')html"' not in html
out = ("/* Generado por embed_html.py a partir de index.html - no editar a mano */\n"
       "#pragma once\n\n"
       f'static const char INDEX_HTML[] = R"html({html})html";\n')
open(os.path.join(DIR, "index_html.h"), "w", encoding="utf-8").write(out)
print(f"index_html.h: {len(html.encode())} bytes")
