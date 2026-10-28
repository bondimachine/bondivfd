/*
 * doom.c - Show inspirado en la barra de estado del Doom, solo configuracion
 * (ver screen.h).
 *
 *   |  50 100% {0}{1}   0% {4}{4}|   {0}{1}{2}{3}: la cara, {4}: llave
 *   |AMMO HLTH {2}{3} ARMR   |
 *
 * La cara ocupa 2x2 caracteres (10x14 puntos) y mira a los costados; solo
 * cambia la mitad de arriba. La secuencia: dispara (baja la municion),
 * recibe dos golpes (baja la salud), levanta armadura y vuelve a disparar.
 * Usa 9 de las 10 ubicaciones de caracteres custom.
 *
 * La cara mirando al centro:
 *     ..######..
 *     .########.
 *     ##......##
 *     #.##..##.#
 *     #........#
 *     #..#...#.#
 *     #...##...#
 *     #........#
 *     #.######.#
 *     #..#..#..#
 *     .#......#.
 *     ..#....#..
 *     ...####...
 *     ..........
 */
#include "doom.h"

#define LOOK_MS 600             /* cada cuanto mira para otro lado */

/* {0} arriba a la izquierda: mira a la izquierda, al centro, a la derecha, al centro */
static const uint8_t face_tl[] = {
    0x07, 0x0F, 0x18, 0x16, 0x10, 0x14, 0x11,
    0x07, 0x0F, 0x18, 0x16, 0x10, 0x12, 0x11,
    0x07, 0x0F, 0x18, 0x16, 0x10, 0x11, 0x11,
    0x07, 0x0F, 0x18, 0x16, 0x10, 0x12, 0x11,
};

/* {1} arriba a la derecha */
static const uint8_t face_tr[] = {
    0x1C, 0x1E, 0x03, 0x0D, 0x01, 0x09, 0x11,
    0x1C, 0x1E, 0x03, 0x0D, 0x01, 0x05, 0x11,
    0x1C, 0x1E, 0x03, 0x0D, 0x01, 0x03, 0x11,
    0x1C, 0x1E, 0x03, 0x0D, 0x01, 0x05, 0x11,
};

/* {2} abajo a la izquierda (boca y menton) */
static const uint8_t face_bl[] = {
    0x10, 0x17, 0x12, 0x08, 0x04, 0x03, 0x00,
};

/* {3} abajo a la derecha */
static const uint8_t face_br[] = {
    0x01, 0x1D, 0x09, 0x02, 0x04, 0x18, 0x00,
};

/* {4} llave */
static const uint8_t key[] = {
    0x0E, 0x11, 0x0E, 0x04, 0x06, 0x04, 0x06,
};

static const screen_glyph_t glyphs[] = {
    { 0, 4, LOOK_MS, face_tl },
    { 1, 4, LOOK_MS, face_tr },
    { 2, 1, 0, face_bl },
    { 3, 1, 0, face_br },
    { 4, 1, 0, key },
};

/* La linea de abajo (etiquetas y menton) se pone en la primera pagina y
 * despues queda igual (NULL); solo se reescriben los numeros. */
#define KEEP     { 0, 0, 0, 0, 0, 0, 0 }
#define LINE(t)  { t, SCREEN_LEFT, SCREEN_STATIC, 0, SCREEN_FX_NONE, 0, 0 }

static const screen_page_t pages[] = {
    { { LINE("  50 100% {0}{1}   0% {4}{4}"), LINE("AMMO HLTH {2}{3} ARMR   ") }, 2000 },
    { { LINE("  49 100% {0}{1}   0% {4}{4}"), KEEP }, 250 },
    { { LINE("  48 100% {0}{1}   0% {4}{4}"), KEEP }, 250 },
    { { LINE("  47 100% {0}{1}   0% {4}{4}"), KEEP }, 1200 },
    { { LINE("  47  83% {0}{1}   0% {4}{4}"), KEEP }, 1500 },
    { { LINE("  47  61% {0}{1}   0% {4}{4}"), KEEP }, 2000 },
    { { LINE("  47  61% {0}{1} 100% {4}{4}"), KEEP }, 2500 },
    { { LINE("  46  61% {0}{1} 100% {4}{4}"), KEEP }, 250 },
    { { LINE("  45  61% {0}{1} 100% {4}{4}"), KEEP }, 250 },
    { { LINE("  44  61% {0}{1} 100% {4}{4}"), KEEP }, 2000 },
};

const screen_show_t doom_show = {
    glyphs, sizeof glyphs / sizeof glyphs[0],
    pages, sizeof pages / sizeof pages[0],
};
