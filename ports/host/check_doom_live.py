#!/usr/bin/env python3
"""Verifica la salida de doom_live_test: al final de cada estado, las dos
lineas y que la cara cargada en el display sea la que dibuja doom_faces.py
para ese st_faceindex.

Uso: ./doom_live_test | python3 check_doom_live.py
"""
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, "..", "..", "tools"))
sys.path.insert(0, os.path.join(HERE, "..", ".."))
import vfd_model  # noqa: E402
import doom_faces  # noqa: E402

# (ms al final de cada estado, linea de arriba, linea de abajo, cara, llaves)
EXPECTED = [
    (1499, "  50 100% ??   0%   ", "AMMO HLTH ?? ARM    ", 1, ""),
    (1999, "     100% ??   0%?  ", "AMMO HLTH ?? ARM B  ", 1, "b"),
    (2499, "  48  83% ??   0%?  ", "AMMO HLTH ?? ARM B  ", 13, "b"),
    (2999, "  48  61% ?? 200%? ?", "AMMO HLTH ?? ARM B R", 22, "br"),
    (3499, "  48  61% ?? 200%? ?", "AMMO HLTH ?? ARM B R", 40, "br"),
    (3999, "  48   0% ?? 200%? ?", "AMMO HLTH ?? ARM B R", 41, "br"),
]
KEY = [".###.", "#...#", ".###.", "..#..", "..##.", "..#..", "..##."]


def main():
    vfd = vfd_model.VFD()
    states = []
    for line in sys.stdin:
        t, b = line.split()
        vfd.byte(int(b, 16), int(t) * 1000.0)
        states.append((int(t), list(vfd.chars), dict(vfd.udc)))

    def at(ms):
        return [s for s in states if s[0] <= ms][-1]

    for ms, top, bottom, face, keys in EXPECTED:
        _, ch, udc = at(ms)
        got_top, got_bottom = vfd_model.render_text(ch[:20]), vfd_model.render_text(ch[20:])
        assert got_top == top, (ms, got_top)
        assert got_bottom == bottom, (ms, got_bottom)
        quarters = [udc[ch[10]], udc[ch[11]], udc[ch[30]], udc[ch[31]]]
        drawn = [quarters[0][r] + quarters[1][r] for r in range(7)] + \
                [quarters[2][r] + quarters[3][r] for r in range(7)]
        assert drawn == doom_faces.full_face(face), (ms, face, drawn)
        for k, col in zip("byr", (17, 18, 19)):
            if k in keys:
                assert udc[ch[col]] == KEY, (ms, k)
    print(f"OK doom en vivo: {len(EXPECTED)} estados, numeros, llaves y caras "
          f"(dolor, ouch, sonrisa, dios, muerto) bien")


if __name__ == "__main__":
    try:
        main()
    except (AssertionError, vfd_model.Fault) as e:
        print(f"FALLA: {e}")
        sys.exit(1)
