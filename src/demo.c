/*
 * demo.c - Show de demostracion, solo configuracion (ver screen.h).
 *
 * Linea superior: el titulo, que se tipea. Linea inferior: Pac-Man pasa
 * hacia la izquierda perseguido por cuatro fantasmas, y despues vuelve hacia
 * la derecha persiguiendolos asustados. Al final, una pantalla con el nombre
 * del display, y vuelve a empezar.
 *
 * Los dibujos estan en glyphs.py. Los frames iguales (Pac-Man cerrado, que
 * comparten las dos direcciones) ocupan una sola ubicacion del display: los
 * 12 frames usan 9 de las 10.
 */
#include "demo.h"
#include "glyphs.h"

#define FRAME_MS 120            /* paso de la animacion */

static const uint8_t pac_left[] = { PAC_L_OPEN, PAC_L_HALF, PAC_CLOSED, PAC_L_HALF };
static const uint8_t pac_right[] = { PAC_R_OPEN, PAC_R_HALF, PAC_CLOSED, PAC_R_HALF };
static const uint8_t ghost[] = { GHOST_A, GHOST_B };
static const uint8_t scared[] = { SCARED_A, SCARED_B };

/* {0} Pac-Man a la izquierda, {1} fantasma, {2} asustado, {3} Pac-Man a la derecha */
static const screen_glyph_t glyphs[] = {
    { 0, 4, FRAME_MS, pac_left },
    { 1, 2, FRAME_MS, ghost },
    { 2, 2, FRAME_MS, scared },
    { 3, 4, FRAME_MS, pac_right },
};

/* Lineas sin texto (NULL) quedan como estaban en la pagina anterior */
#define KEEP   { 0, 0, 0, 0, 0, 0, 0 }
#define EMPTY  { "", SCREEN_LEFT, SCREEN_STATIC, 0, SCREEN_FX_NONE, 0, 0 }
#define CENTER(t) { t, SCREEN_CENTER, SCREEN_STATIC, 0, SCREEN_FX_NONE, 0, 0 }
#define PASS(t, dir) { t, SCREEN_LEFT, dir, FRAME_MS, SCREEN_FX_NONE, 0, 0 }

static const screen_page_t pages[] = {
    { { { "BondiVFD", SCREEN_CENTER, SCREEN_STATIC, 0, SCREEN_FX_TYPE, 100, SCREEN_CURSOR_BLOCK },
        EMPTY }, 0 },
    /* Los fantasmas persiguen a Pac-Man hacia la izquierda */
    { { KEEP, PASS("{0}  {1} {1} {1} {1}", SCREEN_PASS_LEFT) }, 0 },
    { { KEEP, EMPTY }, 600 },
    /* Pac-Man persigue a los fantasmas asustados hacia la derecha */
    { { KEEP, PASS("{3}  {2} {2} {2} {2}", SCREEN_PASS_RIGHT) }, 0 },
    { { KEEP, EMPTY }, 500 },
    { { CENTER("IEE 03601-95B-40"), CENTER("VFD 2x20") }, 4000 },
};

const screen_show_t demo_show = {
    glyphs, sizeof glyphs / sizeof glyphs[0],
    pages, sizeof pages / sizeof pages[0],
};
