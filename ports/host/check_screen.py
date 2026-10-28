#!/usr/bin/env python3
"""Reproduce la salida de screen_test (lineas "ms byte") en el modelo del VFD
de tools/vfd_model.py y verifica alineacion, marquesina, frames animados y que los
caracteres custom cargados sean los definidos.

Uso: ./screen_test | python3 check_screen.py
"""
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "..", "tools"))
import vfd_model  # noqa: E402

PAC = [[0x0E, 0x1E, 0x1C, 0x18, 0x1C, 0x1E, 0x0E],
       [0x0E, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x0E]]
HEART = [0x00, 0x0A, 0x1F, 0x1F, 0x0E, 0x04, 0x00]


def rows_str(rows):
    return ["".join("#" if r & (0x10 >> c) else "." for c in range(5)) for r in rows]


def main():
    events = [(int(t), int(b, 16)) for t, b in (line.split() for line in sys.stdin if line.strip())]
    vfd = vfd_model.VFD()
    states = []                   # (ms, chars, udc) despues de cada byte
    for t, b in events:
        vfd.byte(b, t * 1000.0)
        states.append((t, list(vfd.chars), dict(vfd.udc)))

    def at(ms):
        last = None
        for s in states:
            if s[0] > ms:
                break
            last = s
        return last

    def text(chars):
        return "".join(chr(c) if 0x20 <= c < 0x7F else "~" for c in chars)

    # 1. Linea 0 centrada "Hola " + Pac-Man animado
    seen_frames = set()
    for ms in range(700, 1500, 50):
        _, ch, udc = at(ms)
        assert text(ch[:20]) == "       Hola ~       ", repr(text(ch[:20]))
        rows = udc[ch[12]]
        assert rows in (rows_str(PAC[0]), rows_str(PAC[1])), rows
        seen_frames.add(ch[12])
    assert len(seen_frames) == 2, "Pac-Man no alterno entre sus dos frames"

    # 2. Linea 1 a la derecha: "A" n-tilde "o " corazon " {x}" circulo lleno
    #    (en el display la n-tilde es 0xCE y el circulo 0xDB, no Latin-1)
    _, ch, udc = at(1450)
    bottom = ch[20:]
    assert bottom[10:] == [ord("A"), 0xCE, ord("o"), 0x20, 0xF8, 0x20, ord("{"), ord("x"), ord("}"), 0xDB], bottom
    assert bottom[:10] == [0x20] * 10
    assert udc[0xF8] == rows_str(HEART)

    # 3. Marquesina: cada paso rota una posicion la ventana de 20 sobre
    #    "Coloque monedas $1.40" + 4 espacios
    content = "Coloque monedas $1.40" + " " * 4
    windows = []
    prev = None
    for ms in range(1550, 3000, 10):
        w = text(at(ms)[1][20:])
        if w != prev and len(w) == 20:
            windows.append(w)
            prev = w
    expected = [(content * 2)[k:k + 20] for k in range(len(content))]
    steps = [expected.index(w) for w in windows if w in expected]
    assert len(steps) >= 8, windows
    assert all((b - a) % len(content) == 1 for a, b in zip(steps, steps[1:])), steps

    # 4. Al borrar el Pac-Man, el corazon pasa a la ubicacion 0 (0xF6)
    _, ch, udc = at(3499)
    assert text(ch[:20]) == "izq" + " " * 17, text(ch[:20])
    assert udc[0xF6] == rows_str(HEART)

    # 5. Tipear: "ABCDE" aparece de a una letra, con el bloque (0xEF) adelante
    #    titilando, y al final queda el texto sin cursor
    def distinct(row, t0, t1, step=5):
        out = []
        for ms in range(t0, t1, step):
            ch = at(ms)[1]
            v = ch[row * 20:row * 20 + 20]
            if not out or out[-1] != v:
                out.append(v)
        return out

    def since_first(states, allowed):
        """Descarta lo que quedaba del texto anterior hasta que se redibuja."""
        for i, v in enumerate(states):
            if all(c in allowed for c in v):
                return states[i:]
        return []

    typed = since_first(distinct(0, 3500, 4450), set(b"ABCDE ") | {0xEF})
    shown = []
    for v in typed:
        txt = bytes(c for c in v if c not in (0x20, 0xEF)).decode()
        assert "ABCDE".startswith(txt), v
        k = len(txt)
        assert v[:k] == list(b"ABCDE"[:k]) and all(c == 0x20 for c in v[k + 1:]), v
        assert v[k] in (0x20, 0xEF) if k < 20 else True, v
        shown.append(k)
    assert shown == sorted(shown) and shown[-1] == 5, shown
    assert any(0xEF in v for v in typed), "el bloque no aparecio"
    assert typed[-1] == list(b"ABCDE") + [0x20] * 15, typed[-1]

    # 6. Borrar: Pac-Man (0xF6/0xF7) avanza sobre "12345" (alineado a la
    #    derecha) y deja la linea vacia
    _, _, udc = at(4400)
    erased = since_first(distinct(1, 3500, 4450), set(b"12345 ") | {0xF6, 0xF7})
    pacs = set()
    last_pos = -1
    for v in erased:
        cur = [i for i, c in enumerate(v) if c in (0xF6, 0xF7)]
        if not cur:
            continue
        i = cur[0]
        pacs.add(v[i])
        assert i >= last_pos and all(c == 0x20 for c in v[:i]), v
        assert v[i + 1:] == list(b"12345"[i - 14:]) if i < 19 else True, v
        last_pos = i
    assert last_pos >= 19 and len(pacs) == 2, (last_pos, pacs)
    assert all(udc[c] in (rows_str(PAC[0]), rows_str(PAC[1])) for c in pacs)
    assert erased[-1] == [0x20] * 20, erased[-1]

    # 7. Ciclo sin cursor: "XY" centrado se tipea, se mantiene 2 s, se borra,
    #    queda vacio 1 s y vuelve a empezar
    def line0(ms):
        return text(at(ms)[1][:20])
    assert line0(5500) == " " * 9 + "XY" + " " * 9, line0(5500)
    assert line0(7300) == " " * 20, line0(7300)
    assert any(line0(ms).strip() for ms in range(7700, 8150, 10)), "no volvio a tipear"

    # 8. Marquesina hacia la derecha: cada paso mueve el texto una columna a
    #    la derecha ("Hola mundo" + 10 espacios = vuelta de 20)
    content = "Hola mundo" + " " * 10
    windows = []
    for ms in range(8300, 9790, 10):
        w = text(at(ms)[1][20:])
        if not windows or windows[-1] != w:
            windows.append(w)
    shifts = [(content * 2)[20 - k:40 - k] for k in range(20)]   # k pasos a la derecha
    ks = [shifts.index(w) for w in windows if w in shifts]
    assert len(ks) >= 12, windows
    assert all((b - a) % 20 == 1 for a, b in zip(ks, ks[1:])), ks

    # 9. Show: pagina 1 con "A" arriba y Pac-Man pasando hacia la izquierda;
    #    pagina 2 vacia 300 ms (la linea de arriba queda); pagina 3 con "B" y
    #    Pac-Man pasando hacia la derecha; despues vuelve a la pagina 1. Los
    #    Pac-Man comparten el frame cerrado: 5 ubicaciones para 8 frames.
    from glyphs import GLYPHS
    art = dict(GLYPHS)
    seq = []
    for ms in range(9800, 14000, 5):
        _, ch, udc = at(ms)
        top = text(ch[:20]).strip()
        pac = [(i, udc.get(c)) for i, c in enumerate(ch[20:]) if c >= 0xF6]
        state = (top, pac[0][0] if pac else None)
        if not seq or seq[-1][0] != state:
            seq.append((state, pac[0][1] if pac else None, ms))
    tops = [st[0][0] for st in seq]
    assert "A" in tops and "B" in tops, tops
    a_cols = [st[0][1] for st in seq if st[0][0] == "A" and st[0][1] is not None]
    b_cols = [st[0][1] for st in seq if st[0][0] == "B" and st[0][1] is not None]
    assert a_cols[:5] == sorted(a_cols[:5], reverse=True) and a_cols[0] == 19, a_cols
    assert b_cols[:5] == sorted(b_cols[:5]) and b_cols[0] == 0, b_cols
    left = {"PAC_L_OPEN", "PAC_L_HALF", "PAC_CLOSED"}
    right = {"PAC_R_OPEN", "PAC_R_HALF", "PAC_CLOSED"}
    for st in seq:
        if st[1] is not None:
            name = next(n for n, r in art.items() if r == st[1])
            assert name in (left if st[0][0] == "A" else right), (st, name)
    # despues de "A" viene la pausa (sin Pac-Man, "A" sigue arriba) y despues "B"
    i_b = tops.index("B")
    assert seq[i_b - 1][0] == ("A", None), seq[i_b - 1]
    assert "A" in tops[i_b:], "el show no volvio a la primera pagina"
    codes = {c for t, ch, _u in states if t >= 9800 for c in ch[20:] if c >= 0xF6}
    closed_codes = {c for c in codes if at(13999)[2].get(c) == art["PAC_CLOSED"]}
    assert len(closed_codes) == 1, closed_codes
    assert len(codes) <= 5, sorted(hex(c) for c in codes)

    print(f"OK screen: {len(events)} bytes, Pac-Man alterna, alineacion y acentos bien,"
          f" marquesina {len(steps)} pasos, corazon reubicado en 0xF6, tipear/borrar/ciclo bien,"
          f" marquesina a la derecha, show con pasadas y frames compartidos")


if __name__ == "__main__":
    try:
        main()
    except (AssertionError, vfd_model.Fault) as e:
        print(f"FALLA: {e}")
        sys.exit(1)
