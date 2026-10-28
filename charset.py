#!/usr/bin/env python3
"""Juego de caracteres del VFD IEE 03601-95B-40.

FONT es la fuente 5x7 del display, extraida de la figura 3 de la hoja de
datos (S03601-95B-40, sheet 8): por codigo, 7 filas de 2 digitos hex con
los bits 4..0 = columnas de izquierda a derecha. UNICODE dice que codigo
del display usar para cada caracter que no es ASCII. Ojo: el juego europeo
no es Latin-1, y 0x5C y 0x7C son el yen y la libra.

Genera src/charset.h (para screen.c) y el bloque de la fuente en
examples/BondiWeb/index.html (para la vista previa).
"""
import os
import re

FONT = {
    0x20: "00000000000000", 0x21: "04040404040004", 0x22: "0A0A0A00000000", 0x23: "0A0A1F0A1F0A0A",
    0x24: "040F140E051E04", 0x25: "18190204081303", 0x26: "0814140815120D", 0x27: "04040400000000",
    0x28: "04081010100804", 0x29: "04020101010204", 0x2A: "04150E040E1504", 0x2B: "0004041F040400",
    0x2C: "00000000040408", 0x2D: "0000001F000000", 0x2E: "00000000000004", 0x2F: "01010204081010",
    0x30: "0E11131519110E", 0x31: "040C040404040E", 0x32: "0E11010E10101F", 0x33: "0E11010601110E",
    0x34: "02060A121F0202", 0x35: "1F101E0101110E", 0x36: "0608101E11110E", 0x37: "1F010204080808",
    0x38: "0E11110E11110E", 0x39: "0E11110F01020C", 0x3A: "00000400040000", 0x3B: "00000400040408",
    0x3C: "02040810080402", 0x3D: "00001F001F0000", 0x3E: "08040201020408", 0x3F: "0E110204040004",
    0x40: "0E11171517100E", 0x41: "040A11111F1111", 0x42: "1E09090E09091E", 0x43: "0E11101010110E",
    0x44: "1E09090909091E", 0x45: "1F10101E10101F", 0x46: "1F10101E101010", 0x47: "0E11101013110F",
    0x48: "1111111F111111", 0x49: "0E04040404040E", 0x4A: "0101010101110E", 0x4B: "11121418141211",
    0x4C: "1010101010101F", 0x4D: "111B1511111111", 0x4E: "11111915131111", 0x4F: "0E11111111110E",
    0x50: "1E11111E101010", 0x51: "0E11111115120D", 0x52: "1E11111E141211", 0x53: "0E11100E01110E",
    0x54: "1F040404040404", 0x55: "1111111111110E", 0x56: "1111110A0A0404", 0x57: "11111115151B11",
    0x58: "11110A040A1111", 0x59: "11110A04040404", 0x5A: "1F01020408101F", 0x5B: "1F18181818181F",
    0x5C: "110A1F041F0404", 0x5D: "1F03030303031F", 0x5E: "0000040A110000", 0x5F: "0000000000001F",
    0x60: "00100804000000", 0x61: "00000E010F110F", 0x62: "1010101E11111E", 0x63: "00000F1010100F",
    0x64: "0101010F11110F", 0x65: "00000E111E100E", 0x66: "0205040E040404", 0x67: "00000E110F010E",
    0x68: "1010101E111111", 0x69: "00040004040404", 0x6A: "01000101010906", 0x6B: "0808090A0C0A09",
    0x6C: "0C04040404040E", 0x6D: "00001B15151511", 0x6E: "00001619111111", 0x6F: "00000E1111110E",
    0x70: "00001E111E1010", 0x71: "00000F110F0101", 0x72: "00001619101010", 0x73: "00000F100E011E",
    0x74: "04041F04040404", 0x75: "0000111111110E", 0x76: "00001111110A04", 0x77: "0000111115150A",
    0x78: "0000110A040A11", 0x79: "0000110A040408", 0x7A: "00001F0204081F", 0x7B: "06080810080806",
    0x7C: "0609081C08081F", 0x7D: "0C02020102020C", 0x7E: "1C141C00000000", 0x7F: "150A150A150A15",
    0xA0: "0A000E111F1111", 0xA1: "04000E111F1111", 0xA2: "02040E111F1111", 0xA3: "0F1010100F0408",
    0xA4: "040A0E111F1111", 0xA5: "070C141F141417", 0xA6: "02041F101E101F", 0xA7: "08041F101E101F",
    0xA8: "040A1F101E101F", 0xA9: "0A000E0404040E", 0xAA: "02040E0404040E", 0xAB: "08040E0404040E",
    0xAC: "040A0E0404040E", 0xAD: "0A000E1111110E", 0xAE: "0D160019151311", 0xAF: "0E11131519110E",
    0xB0: "02040E1111110E", 0xB1: "08040E1111110E", 0xB2: "08040E111F1111", 0xB3: "1E111112111112",
    0xB4: "0A001F101E101F", 0xB5: "0A00111111110E", 0xB6: "0204111111110E", 0xB7: "0804111111110E",
    0xB8: "040A001111110E", 0xB9: "040A0400000000", 0xBA: "04041F04041F00", 0xBB: "0004001F000400",
    0xBC: "00041E1F1E0400", 0xBD: "00041E111E0400", 0xBE: "00040E1F0E0400", 0xBF: "040A0E1111110E",
    0xC0: "0A000E010F110F", 0xC1: "04000E010F110F", 0xC2: "02040E010F110F", 0xC3: "000E10100E0408",
    0xC4: "040A0E010F110F", 0xC5: "00001A051F140B", 0xC6: "02040E111F100F", 0xC7: "08040E111F100F",
    0xC8: "040A0E111F100F", 0xC9: "000A0004040404", 0xCA: "02040004040404", 0xCB: "08040004040404",
    0xCC: "040A0004040404", 0xCD: "000A000E0A0A0E", 0xCE: "0D160016191111", 0xCF: "00000E1315190E",
    0xD0: "0204000E0A0A0E", 0xD1: "0804000E0A0A0E", 0xD2: "08040E010F110F", 0xD3: "00040E14140E04",
    0xD4: "0A000E111F100F", 0xD5: "000A000A0A0A0E", 0xD6: "0204000A0A0A0E", 0xD7: "0804000A0A0A0E",
    0xD8: "040A000A0A0A0E", 0xD9: "000E1111110E00", 0xDA: "000E1115110E00", 0xDB: "000E1F1F1F0E00",
    0xDC: "001F1111111F00", 0xDD: "001F1F1F1F1F00", 0xDE: "00040A110A0400", 0xDF: "040A000E0A0A0E",
    0xE0: "1F11151515111F", 0xE1: "1F1B131B1B111F", 0xE2: "1F131D1B17111F", 0xE3: "1F131D1B1D131F",
    0xE4: "1F1D1915111D1F", 0xE5: "1F1117131D131F", 0xE6: "1F17171115111F", 0xE7: "1F111D1B17171F",
    0xE8: "1F11151115111F", 0xE9: "1F1115111D1D1F", 0xEA: "00000000000000", 0xEB: "00000000000000",
    0xEC: "00000000000000", 0xED: "00000000000000", 0xEE: "00000000000000", 0xEF: "1F1F1F1F1F1F1F",}

UNICODE = {
    "¥": 0x5C, "£": 0x7C,
    "Ä": 0xA0, "Å": 0xA1, "Á": 0xA2, "Ç": 0xA3, "Â": 0xA4, "Æ": 0xA5, "É": 0xA6, "È": 0xA7,
    "Ê": 0xA8, "Ï": 0xA9, "Í": 0xAA, "Ì": 0xAB, "Î": 0xAC, "Ö": 0xAD, "Ñ": 0xAE, "Ø": 0xAF,
    "Ó": 0xB0, "Ò": 0xB1, "À": 0xB2, "ß": 0xB3, "Ë": 0xB4, "Ü": 0xB5, "Ú": 0xB6, "Ù": 0xB7,
    "Û": 0xB8, "°": 0xB9, "±": 0xBA, "÷": 0xBB, "→": 0xBC, "⇒": 0xBD, "◆": 0xBE, "Ô": 0xBF,
    "ä": 0xC0, "å": 0xC1, "á": 0xC2, "ç": 0xC3, "â": 0xC4, "æ": 0xC5, "é": 0xC6, "è": 0xC7,
    "ê": 0xC8, "ï": 0xC9, "í": 0xCA, "ì": 0xCB, "î": 0xCC, "ö": 0xCD, "ñ": 0xCE, "ø": 0xCF,
    "ó": 0xD0, "ò": 0xD1, "à": 0xD2, "¢": 0xD3, "ë": 0xD4, "ü": 0xD5, "ú": 0xD6, "ù": 0xD7,
    "û": 0xD8, "○": 0xD9, "◉": 0xDA, "●": 0xDB, "□": 0xDC, "■": 0xDD, "◇": 0xDE, "ô": 0xDF,
    "█": 0xEF,
}

HERE = os.path.dirname(os.path.abspath(__file__))


def write_c():
    out = ["/* Generado por charset.py - no editar a mano */",
           "#ifndef CHARSET_H", "#define CHARSET_H", "", "#include <stdint.h>", "",
           "/* Unicode -> codigo del display, ordenado por Unicode */",
           "typedef struct { uint16_t unicode; uint8_t code; } charset_entry_t;", "",
           f"#define CHARSET_LEN {len(UNICODE)}", "",
           "#ifdef CHARSET_DEFINE_DATA",
           "static const charset_entry_t charset[CHARSET_LEN] = {"]
    for ch, code in sorted(UNICODE.items(), key=lambda kv: ord(kv[0])):
        out.append(f"    {{ 0x{ord(ch):04X}, 0x{code:02X} }},  /* {ch} */")
    out += ["};", "#endif", "", "#endif", ""]
    open(os.path.join(HERE, "src", "charset.h"), "w", encoding="utf-8").write("\n".join(out))


def write_js():
    path = os.path.join(HERE, "examples", "BondiWeb", "index.html")
    html = open(path, encoding="utf-8").read()
    font = ",".join(f'"{c:02X}":"{rows}"' for c, rows in FONT.items())
    uni = ",".join(f'"{ch}":{code}' for ch, code in UNICODE.items())
    block = ("/* charset:begin - generado por charset.py */\n"
             f"const FONT = {{{font}}};\n"
             f"const UNI = {{{uni}}};\n"
             "/* charset:end */")
    html, n = re.subn(r"/\* charset:begin.*?charset:end \*/", lambda m: block, html, flags=re.S)
    assert n == 1, "falta el bloque charset en index.html"
    open(path, "w", encoding="utf-8").write(html)


if __name__ == "__main__":
    assert all(0x20 <= c < 0x80 or 0xA0 <= c < 0xF0 for c in FONT)
    assert all(code in FONT for code in UNICODE.values())
    write_c()
    write_js()
