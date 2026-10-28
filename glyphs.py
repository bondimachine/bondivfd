#!/usr/bin/env python3
"""Caracteres custom (UDC) del VFD IEE 03601-95B-40: el mapeo de puntos de
la carga (hoja de datos sec. 4.6) y los dibujos de Pac-Man de la demo. Genera
src/glyphs.h."""
import os

# Bit de cada byte de dot data -> numero de punto de la matriz 5x7
# (1..35, de izquierda a derecha y de arriba hacia abajo). Hoja de datos
# S03601-95B-40, sheet 9, "CHARACTER DOT DATA". El bit 6 del ultimo byte es
# el bit E (continuar cargando otro caracter).
DOT_MAP = [
    {6: 3, 5: 17, 4: 34},                                   # byte 3
    {6: 7, 5: 13, 4: 30, 3: 23, 2: 4, 1: 14, 0: 33},        # byte 4
    {6: 11, 5: 9, 4: 26, 3: 27, 2: 8, 1: 10, 0: 29},        # byte 5
    {6: 15, 5: 5, 4: 22, 3: 31, 2: 12, 1: 6, 0: 25},        # byte 6
    {6: 19, 5: 1, 4: 18, 3: 35, 2: 16, 1: 2, 0: 21},        # byte 7
    {3: 32, 2: 28, 1: 24, 0: 20},                           # byte 8
]
END_BIT = 0x40
assert sorted(d for b in DOT_MAP for d in b.values()) == list(range(1, 36))


def mirror(rows):
    return [r[::-1] for r in rows]


PAC_R_OPEN = [".###.", "####.", "###..", "##...", "###..", "####.", ".###."]
PAC_R_HALF = [".###.", "#####", "####.", "##...", "####.", "#####", ".###."]

GLYPHS = [
    ("PAC_R_OPEN", PAC_R_OPEN),
    ("PAC_R_HALF", PAC_R_HALF),
    ("PAC_CLOSED", [".###.", "#####", "#####", "#####", "#####", "#####", ".###."]),
    ("PAC_L_OPEN", mirror(PAC_R_OPEN)),
    ("PAC_L_HALF", mirror(PAC_R_HALF)),
    ("GHOST_A",    [".###.", "#####", "#.#.#", "#####", "#####", "#####", "#.#.#"]),
    ("GHOST_B",    [".###.", "#####", "#.#.#", "#####", "#####", "#####", ".#.#."]),
    ("SCARED_A",   [".###.", "#...#", "##.##", "#...#", "#.#.#", "##.##", "#.#.#"]),
    ("SCARED_B",   [".###.", "#...#", "##.##", "#...#", "##.##", "#.#.#", ".#.#."]),
]


def encode(rows):
    assert len(rows) == 7 and all(len(r) == 5 for r in rows)
    lit = {i + 1 for i, ch in enumerate("".join(rows)) if ch == "#"}
    return [sum(1 << bit for bit, dot in b.items() if dot in lit) for b in DOT_MAP]


def decode(data):
    lit = {dot for byte, b in zip(data, DOT_MAP) for bit, dot in b.items() if byte >> bit & 1}
    return ["".join("#" if r * 5 + c + 1 in lit else "." for c in range(5)) for r in range(7)]


def rows_bits(rows):
    return [int(r.replace("#", "1").replace(".", "0"), 2) for r in rows]


def main():
    out = ["/* Generado por glyphs.py - no editar a mano */",
           "#ifndef GLYPHS_H", "#define GLYPHS_H", "", "#include <stdint.h>", "",
           "/* Dibujos de Pac-Man y los fantasmas: 7 filas, bits 4..0 = columnas de",
           " * izquierda a derecha (formato de screen_glyph_t) */"]
    for name, rows in GLYPHS:
        assert decode(encode(rows)) == rows
        out.append(f"#define {name:<11} " + ", ".join(f"0x{b:02X}" for b in rows_bits(rows)))
    out += ["",
            "/* Punto n (0..34, fila por fila de izquierda a derecha) -> byte de dot",
            " * data (0..5) en el nibble alto y bit en el bajo. Para vfd_char.c. */",
            "#ifdef GLYPHS_DEFINE_DOT_MAP",
            "static const uint8_t dot_map[35] = {"]
    where = {dot: (i << 4) | bit for i, b in enumerate(DOT_MAP) for bit, dot in b.items()}
    for r in range(7):
        out.append("    " + ", ".join(f"0x{where[r * 5 + c + 1]:02X}" for c in range(5)) + ",")
    out += ["};", "#endif", "", "#endif", ""]
    path = os.path.join(os.path.dirname(os.path.abspath(__file__)), "src", "glyphs.h")
    open(path, "w").write("\n".join(out))


if __name__ == "__main__":
    main()
