/*
 * screen_test.c - Maneja screen.c con un reloj simulado y escribe cada byte
 * que recibiria el VFD como "ms byte" (hex). check_screen.py lo reproduce en
 * el modelo del display (tools/vfd_model.py) y verifica el resultado.
 */
#include <stdio.h>
#include <stdint.h>
#include "vfd.h"
#include "screen.h"
#include "glyphs.h"

extern uint16_t host_ms;

static const uint8_t pac[2][7] = {
    { 0x0E, 0x1E, 0x1C, 0x18, 0x1C, 0x1E, 0x0E },     /* abierto */
    { 0x0E, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x0E },     /* cerrado */
};
static const uint8_t heart[7] = { 0x00, 0x0A, 0x1F, 0x1F, 0x0E, 0x04, 0x00 };

static void run_until(uint16_t ms)
{
    for (; host_ms < ms; host_ms++) {
        vfd_poll();
        screen_poll();
    }
}

static void line(uint8_t n, const char *text, uint8_t align, uint8_t scroll, uint16_t ms)
{
    screen_line_t l = { text, align, scroll, ms, SCREEN_FX_NONE, 0, 0 };
    screen_set_line(n, &l);
}

static void effect(uint8_t n, const char *text, uint8_t align, uint8_t fx, uint16_t ms, uint8_t cursor)
{
    screen_line_t l = { text, align, SCREEN_STATIC, 0, fx, ms, cursor };
    screen_set_line(n, &l);
}

/* Show de prueba: Pac-Man pasa hacia la izquierda, pausa, y vuelve hacia la
 * derecha. Los dos Pac-Man comparten el frame cerrado. */
static const uint8_t pac_l[] = { PAC_L_OPEN, PAC_L_HALF, PAC_CLOSED, PAC_L_HALF };
static const uint8_t pac_r[] = { PAC_R_OPEN, PAC_R_HALF, PAC_CLOSED, PAC_R_HALF };
static const screen_glyph_t show_glyphs[] = {
    { 0, 4, 50, pac_l },
    { 1, 4, 50, pac_r },
};
static const screen_page_t show_pages[] = {
    { { { "A", SCREEN_CENTER, 0, 0, 0, 0, 0 }, { "{0}AB", SCREEN_LEFT, SCREEN_PASS_LEFT, 50, 0, 0, 0 } }, 0 },
    { { { 0, 0, 0, 0, 0, 0, 0 }, { "", SCREEN_LEFT, 0, 0, 0, 0, 0 } }, 300 },
    { { { "B", SCREEN_CENTER, 0, 0, 0, 0, 0 }, { "{1}", SCREEN_LEFT, SCREEN_PASS_RIGHT, 50, 0, 0, 0 } }, 0 },
};
static const screen_show_t test_show = { show_glyphs, 2, show_pages, 3 };

int main(void)
{
    vfd_begin();
    screen_begin();
    screen_set_glyph(0, 2, &pac[0][0], 200);
    screen_set_glyph(1, 1, heart, 0);
    line(0, "Hola {0}", SCREEN_CENTER, SCREEN_STATIC, 0);
    line(1, "A\xc3\xb1o {1} {{x}\xe2\x97\x8f", SCREEN_RIGHT, SCREEN_STATIC, 0);
    run_until(1500);
    line(1, "Coloque monedas $1.40", SCREEN_LEFT, SCREEN_SCROLL_LEFT, 150);
    run_until(3000);
    line(0, "izq", SCREEN_LEFT, SCREEN_STATIC, 0);
    screen_set_glyph(0, 0, 0, 0);          /* el corazon pasa a la ubicacion 0 */
    run_until(3500);

    /* Efectos: tipear con bloque que titila, Pac-Man que se come el texto */
    screen_set_glyph(0, 2, &pac[0][0], 200);
    effect(0, "ABCDE", SCREEN_LEFT, SCREEN_FX_TYPE, 100, SCREEN_CURSOR_BLOCK);
    effect(1, "12345", SCREEN_RIGHT, SCREEN_FX_ERASE, 100, 0);
    run_until(4500);

    /* Ciclo: tipea, espera, borra, espera, vuelve a tipear */
    effect(0, "XY", SCREEN_CENTER, SCREEN_FX_CYCLE, 50, SCREEN_CURSOR_NONE);
    run_until(8200);

    /* Marquesina hacia la derecha */
    line(1, "Hola mundo", SCREEN_LEFT, SCREEN_SCROLL_RIGHT, 100);
    run_until(9800);

    /* Show: pasadas de una vez y paginas */
    screen_play(&test_show);
    run_until(14000);
    return 0;
}
