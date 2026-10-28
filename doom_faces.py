#!/usr/bin/env python3
"""Caras del Doomguy para el modo en vivo de la barra del Doom (doom_live.c).

Cada cara es de 10x14 puntos: 2x2 caracteres de 5x7. La mitad de arriba
(pelo, cejas, ojos, nariz) depende del nivel de dolor (0-4, sangre que se va
sumando) y de los ojos; la de abajo, de la boca. Genera src/doom_faces.h.

El indice de cara es el st_faceindex de Doom (st_stuff.c): 8 caras por nivel
de dolor (3 de frente, mirando a la derecha, a la izquierda, ouch, sonrisa
malvada, furia), despues la de modo dios (40) y la de muerto (41).
"""
import os

HAIR = ["..######..",
        ".########.",
        "##......##",
        "#.##..##.#"]           # cejas
EYES = {                        # filas 4 y 5 de la cara
    "left":   ["#........#", "#.#...#..#"],
    "center": ["#........#", "#..#...#.#"],
    "right":  ["#........#", "#...#...##"],
    "wide":   ["#.##..##.#", "#.##..##.#"],
}
NOSE = "#...##...#"
GOD = ["..######..",
       ".########.",
       "##......##",
       "#........#",            # cejas levantadas
       "#.##..##.#",
       "#.##..##.#",
       NOSE]
DEAD = ["..######..",
        ".########.",
        "##......##",
        "##.#..#.##",           # ojos en X
        "#.#....#.#",
        "##.#..#.##",
        NOSE]
MOUTHS = {
    "grimace": ["#........#",
                "#.######.#",
                "#..#..#..#",
                ".#......#.",
                "..#....#..",
                "...####...",
                ".........."],
    "grin":    ["#........#",
                "#.#....#.#",
                "#..####..#",
                ".#......#.",
                "..#....#..",
                "...####...",
                ".........."],
    "ouch":    ["#...##...#",
                "#..#..#..#",
                "#..#..#..#",
                ".#..##..#.",
                "..#....#..",
                "...####...",
                ".........."],
    "dead":    ["#........#",
                "#..####..#",
                "#........#",
                ".#......#.",
                "..#....#..",
                "...####...",
                ".........."],
}
# Sangre que se suma por nivel de dolor: (fila, columna) en la cara de 10x14
BLOOD = [
    [],
    [(2, 6)],
    [(4, 1), (6, 8)],
    [(2, 3), (4, 8), (8, 1), (9, 8)],
    [(2, 4), (5, 5), (10, 3), (11, 6), (6, 1)],
]


def face(pain, top, mouth):
    """Cara completa (14 filas de 10) con la sangre de los niveles 1..pain."""
    rows = [list(r) for r in top + MOUTHS[mouth]]
    for level in range(1, pain + 1):
        for r, c in BLOOD[level]:
            rows[r][c] = "#"
    return ["".join(r) for r in rows]


def top_rows(pain, eyes):
    return face(pain, HAIR + EYES[eyes] + [NOSE], "grimace")[:7]


def bottom_rows(pain, mouth):
    return face(pain, HAIR + EYES["center"] + [NOSE], mouth)[7:]


def quarter(rows, side):
    return [r[side * 5:side * 5 + 5] for r in rows]


def bits(rows):
    return [int(r.replace("#", "1").replace(".", "0"), 2) for r in rows]


EYE_ORDER = ["left", "center", "right", "wide"]
MOUTH_ORDER = ["grimace", "grin", "ouch", "dead"]


def face_index(index):
    """st_faceindex -> (dolor, ojos o "god"/"dead", boca), como lo dibuja doom_live.c"""
    if index == 40:
        return 0, "god", "grimace"
    if index >= 41:
        return 4, "dead", "dead"
    pain, kind = divmod(index, 8)
    eyes, mouth = [("left", "grimace"), ("center", "grimace"), ("right", "grimace"),
                   ("right", "grimace"), ("left", "grimace"), ("wide", "ouch"),
                   ("center", "grin"), ("wide", "grimace")][kind]
    return pain, eyes, mouth


def full_face(index):
    pain, eyes, mouth = face_index(index)
    top = GOD if eyes == "god" else DEAD if eyes == "dead" else HAIR + EYES[eyes] + [NOSE]
    return face(pain, top, mouth)


def main():
    def arr(quarters, dims):
        """Inicializador C con llaves anidadas: dims = tamanos de las
        dimensiones de afuera (la ultima, de 7 filas, va en cada linea)."""
        rows = ["{ " + ", ".join(f"0x{b:02X}" for b in bits(q)) + " }" for q in quarters]
        for d in reversed(dims[1:]):
            rows = ["{ " + ", ".join(rows[i:i + d]) + " }" for i in range(0, len(rows), d)]
        return ",\n".join("    " + r for r in rows)

    tops = [quarter(top_rows(p, e), s) for p in range(5) for e in EYE_ORDER for s in (0, 1)]
    special = [quarter(GOD, s) for s in (0, 1)] + [quarter(face(4, DEAD, "dead")[:7], s) for s in (0, 1)]
    bottoms = [quarter(bottom_rows(p, m), s) for p in range(5) for m in MOUTH_ORDER for s in (0, 1)]
    out = f"""/* Generado por doom_faces.py - no editar a mano */
#ifndef DOOM_FACES_H
#define DOOM_FACES_H

#include <stdint.h>

/* Mitad de arriba de la cara: [dolor 0-4][ojos: izq, centro, der, abiertos]
 * [lado: izq, der][7 filas] */
static const uint8_t doom_face_top[5][4][2][7] = {{
{arr(tops, [5, 4, 2])}
}};

/* Mitad de arriba en modo dios y muerto: [god, dead][lado][7 filas] */
static const uint8_t doom_face_special[2][2][7] = {{
{arr(special, [2, 2])}
}};

/* Mitad de abajo: [dolor 0-4][boca: apretada, sonrisa, ouch, muerto]
 * [lado][7 filas] */
static const uint8_t doom_face_bottom[5][4][2][7] = {{
{arr(bottoms, [5, 4, 2])}
}};

#endif
"""
    path = os.path.join(os.path.dirname(os.path.abspath(__file__)), "src", "doom_faces.h")
    open(path, "w").write(out)


if __name__ == "__main__":
    main()
