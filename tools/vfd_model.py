#!/usr/bin/env python3
"""Modelo del VFD IEE 03601-95B-40 para las pruebas: interpreta los bytes que
recibiria el display (comandos, caracteres custom, tiempos de ejecucion) y
guarda lo que se ve. Tambien verifica la historia de la demo de Pac-Man.

Uso como programa: python3 vfd_model.py [--doom] stream.txt
  stream.txt: salida de ports/host/vfd_host ("ms byte" por linea). Verifica
  que la demo (src/demo.c) cuente bien su historia, o con --doom la barra
  del Doom (src/doom.c).
"""
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), ".."))
import glyphs  # noqa: E402


class Fault(Exception):
    pass



EXEC_US = {"char": 220, "ctrl": 230, "clear": 350, "lf": 230, "reset": 710}
DEFINED_CTRL = {0x08, 0x09, 0x0A, 0x0D, 0x0E, 0x0F, 0x11, 0x12, 0x13, 0x14,
                0x15, 0x16, 0x18, 0x19, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F}


class VFD:
    def __init__(self):
        self.reset_state()
        self.udc = {}
        self.snapshots = []           # (t_us, 40 chars) antes de cada GOTO o CLEAR
        self.writes = 0
        self.log = []                 # todos los bytes recibidos, en orden

    def reset_state(self):
        self.chars = [0x20] * 40
        self.cursor = 0
        self.pending = None           # estado de comandos multi-byte
        self.udc = {}

    def byte(self, b, t):
        """Procesa un byte y devuelve el tiempo de ejecucion en us."""
        self.writes += 1
        self.log.append(b)
        p = self.pending
        if p is not None:
            kind = p[0]
            if kind == "goto":
                self.pending = None
                if b <= 0x27:
                    self.cursor = b
                return EXEC_US["ctrl"]
            if kind == "udc_loc":
                if b > 9:
                    raise Fault(f"ubicacion UDC invalida {b:02X}")
                self.pending = ("udc_data", b, [])
                return EXEC_US["ctrl"]
            if kind == "udc_data":
                p[2].append(b)
                if len(p[2]) == 6:
                    self.udc[0xF6 + p[1]] = glyphs.decode([d & ~glyphs.END_BIT if i == 5 else d
                                                           for i, d in enumerate(p[2])])
                    self.pending = ("udc_loc",) if b & glyphs.END_BIT else None
                return EXEC_US["ctrl"]
            if kind == "bit7":
                self.pending = None
                b |= 0x80
        if b < 0x20 or 0x80 <= b < 0xA0:
            if b not in DEFINED_CTRL:
                raise Fault(f"codigo de control no definido {b:02X}")
            if b == 0x1B:
                self.snapshots.append((t, list(self.chars)))
                self.pending = ("goto",)
            elif b == 0x18:
                self.pending = ("udc_loc",)
            elif b == 0x19:
                self.pending = ("bit7",)
            elif b == 0x15:
                self.snapshots.append((t, list(self.chars)))
                self.chars = [0x20] * 40
                self.cursor = 0
                return EXEC_US["clear"]
            elif b == 0x16:
                self.cursor = 0
            elif b == 0x14:
                self.reset_state()
                return EXEC_US["reset"]
            elif b in (0x12, 0x13, 0x0A, 0x08, 0x09, 0x0D):
                raise Fault(f"comando {b:02X} no modelado")
            return EXEC_US["ctrl"]
        if b >= 0xF6 and b not in self.udc:
            raise Fault(f"caracter custom {b:02X} usado sin cargar")
        self.chars[self.cursor] = b
        self.cursor = (self.cursor + 1) % 40          # modo 0x11: wrap a home
        return EXEC_US["char"]



def render_text(chars):
    return "".join(chr(c) if 0x20 <= c < 0x7F else "?" for c in chars)



GLYPH_ROWS = {tuple(rows): name for name, rows in glyphs.GLYPHS}
PAC_LEFT = {"PAC_L_OPEN", "PAC_L_HALF", "PAC_CLOSED"}
PAC_RIGHT = {"PAC_R_OPEN", "PAC_R_HALF", "PAC_CLOSED"}


def check_story(frames, udc, top_line, final_screen, final_name="final"):
    """Comprueba la historia de la demo de Pac-Man (o el modo atraccion de la
    BondiMachine): con top_line arriba, los fantasmas persiguen a Pac-Man hacia
    la izquierda, despues Pac-Man persigue a los fantasmas asustados hacia la
    derecha, y al final se ve final_screen (las dos lineas). Los personajes se
    reconocen por el dibujo cargado en el display, no por el codigo (las
    ubicaciones las elige screen.c). Devuelve la lista de fases."""
    def name(c):
        return GLYPH_ROWS.get(tuple(udc[c])) if c in udc else None

    phases = []
    prev_pac = None
    for _, chars in frames:
        top, bottom = render_text(chars[:20]), chars[20:]
        names = [name(c) for c in bottom]
        if top != top_line:
            prev_pac = None
            if (top, render_text(bottom)) == final_screen:
                phase = final_name
            elif not any(names):
                continue                      # cambiando de pantalla o tipeandose
            else:
                raise Fault(f"personajes con otra linea arriba: {top!r}")
            if not phases or phases[-1] != phase:
                phases.append(phase)
            continue
        pac = [i for i, n in enumerate(names) if n and n.startswith("PAC")]
        ghosts = [i for i, n in enumerate(names) if n and n.startswith("GHOST")]
        scared = [i for i, n in enumerate(names) if n and n.startswith("SCARED")]
        if ghosts and scared:
            raise Fault("fantasmas normales y asustados en el mismo cuadro")
        phase = "huida" if ghosts else "persecucion" if scared else None
        if pac and phase == "huida":
            assert all(g > pac[0] for g in ghosts), "en la huida los fantasmas van detras (a la derecha)"
            assert names[pac[0]] in PAC_LEFT, "Pac-Man debe mirar a la izquierda"
            if prev_pac is not None and prev_pac[0] == "huida":
                assert pac[0] in (prev_pac[1], prev_pac[1] - 1), "Pac-Man avanza de a 1 columna a la izquierda"
        if pac and phase == "persecucion":
            assert all(g > pac[0] for g in scared), "Pac-Man va detras de los fantasmas"
            assert names[pac[0]] in PAC_RIGHT, "Pac-Man debe mirar a la derecha"
            if prev_pac is not None and prev_pac[0] == "persecucion":
                assert pac[0] in (prev_pac[1], prev_pac[1] + 1), "Pac-Man avanza de a 1 columna a la derecha"
        prev_pac = (phase, pac[0]) if pac else None
        if phase and (not phases or phases[-1] != phase):
            phases.append(phase)
    return phases


def replay(path):
    """Reproduce una salida "ms byte" en el modelo; devuelve el VFD."""
    vfd = VFD()
    for line in open(path):
        if line.strip():
            t, b = line.split()
            vfd.byte(int(b, 16), int(t) * 1000.0)
    return vfd


DEMO_TOP = "      BondiVFD      "
DEMO_FINAL = ("  IEE 03601-95B-40  ", "      VFD 2x20      ")


DOOM_LABELS = "AMMO HLTH ?? ARMR   "
DOOM_STORY = [(50, 100, 0), (49, 100, 0), (48, 100, 0), (47, 100, 0), (47, 83, 0),
              (47, 61, 0), (47, 61, 100), (46, 61, 100), (45, 61, 100), (44, 61, 100)]


def check_doom(vfd):
    """Barra del Doom: la linea de abajo queda fija, los numeros pasan en
    orden, la cara (2x2 caracteres) mira a tres lados y hay 9 ubicaciones."""
    import re
    seen, looks = [], set()
    for _, ch in vfd.snapshots[1:]:
        top, bottom = render_text(ch[:20]), render_text(ch[20:])
        if not bottom.strip():
            continue                          # todavia no se dibujo
        assert bottom == DOOM_LABELS, repr(bottom)
        m = re.fullmatch(r" *(\d+) +(\d+)% \?\? +(\d+)% \?\?", top)
        if not m:
            continue                          # la linea de arriba todavia no se dibujo
        values = tuple(int(g) for g in m.groups())
        if not seen or seen[-1] != values:
            seen.append(values)
        looks.add((tuple(vfd.udc[ch[10]]), tuple(vfd.udc[ch[11]])))
        assert ch[18] == ch[19], "las dos llaves tienen que ser iguales"
    assert seen[:len(DOOM_STORY) + 1] == DOOM_STORY + DOOM_STORY[:1], seen
    assert len(looks) == 3, f"la cara tiene que mirar a 3 lados, miro a {len(looks)}"
    used = {c for _, ch in vfd.snapshots[1:] for c in ch if c >= 0xF6}
    assert len(used) == 9, sorted(hex(c) for c in used)
    print(f"OK doom: {vfd.writes} bytes al VFD, {len(seen)} cambios de numeros, la cara mira a 3 lados, "
          f"{len(used)} ubicaciones")


def main():
    if sys.argv[1] == "--doom":
        return check_doom(replay(sys.argv[2]))
    vfd = replay(sys.argv[1])
    phases = check_story(vfd.snapshots[1:], vfd.udc, DEMO_TOP, DEMO_FINAL)
    assert phases[:6] == ["huida", "persecucion", "final"] * 2, phases
    print(f"OK demo: {vfd.writes} bytes al VFD, fases: {' -> '.join(phases)}")


if __name__ == "__main__":
    try:
        main()
    except (Fault, AssertionError) as e:
        print(f"FALLA: {e}")
        sys.exit(1)
