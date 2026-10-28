/*
 * screen.h - Pantallas de mensajes, portable y no bloqueante, sobre vfd.h.
 *
 * Cada linea tiene texto, alineacion, desplazamiento (marquesina en loop o
 * una sola pasada, hacia la izquierda o la derecha) y un efecto de tipear o
 * borrar. Hasta 10 caracteres custom, {0}..{9} en el texto, con varios
 * frames que se alternan solos.
 *
 * Un "show" es la lista de caracteres custom mas una secuencia de paginas
 * (las dos lineas y cuanto duran) que se repite. El modo atraccion de la
 * BondiMachine es un show, y demo.c tambien.
 *
 * Los textos y los frames NO se copian: tienen que seguir existiendo
 * mientras se muestran (por ejemplo, const).
 */
#ifndef SCREEN_H
#define SCREEN_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SCREEN_LINES     2
#ifndef SCREEN_GLYPHS
#define SCREEN_GLYPHS    10     /* caracteres custom {0}..{N-1}, maximo 10 */
#endif
#define SCREEN_TEXT_MAX  200    /* caracteres por linea que se tienen en cuenta */
#define SCREEN_GAP       4      /* espacios entre vueltas de la marquesina */

enum { SCREEN_LEFT, SCREEN_CENTER, SCREEN_RIGHT };

/* Desplazamiento. SCROLL: marquesina que da vueltas. PASS: el texto entra
 * por un costado, sale por el otro y la linea queda vacia. LEFT/RIGHT es
 * hacia donde se mueve el texto. */
enum {
    SCREEN_STATIC,
    SCREEN_SCROLL_LEFT, SCREEN_SCROLL_RIGHT,
    SCREEN_PASS_LEFT, SCREEN_PASS_RIGHT
};

/* Efectos (solo en lineas SCREEN_STATIC). TYPE: el texto aparece de a un
 * caracter, con el cursor adelante. ERASE: desaparece de izquierda a
 * derecha, con el cursor "comiendolo". CYCLE: tipea, espera SCREEN_HOLD_MS,
 * borra, espera SCREEN_GAP_MS y vuelve a empezar. */
enum { SCREEN_FX_NONE, SCREEN_FX_TYPE, SCREEN_FX_ERASE, SCREEN_FX_CYCLE };
#define SCREEN_CURSOR_BLOCK 0xFF    /* bloque que titila */
#define SCREEN_CURSOR_NONE  0xFE    /* o un caracter custom 0-9 */
#define SCREEN_HOLD_MS      2000
#define SCREEN_GAP_MS       1000
#define SCREEN_BLINK_MS     300

typedef struct {
    const char *text;       /* UTF-8; "{n}" = caracter custom n, "{{" = llave.
                             * En una pagina, NULL deja la linea como estaba. */
    uint8_t align;          /* SCREEN_LEFT/CENTER/RIGHT (lineas fijas) */
    uint8_t scroll;         /* SCREEN_STATIC, SCREEN_SCROLL_*, SCREEN_PASS_* */
    uint16_t scroll_ms;     /* ms por paso de desplazamiento */
    uint8_t effect;         /* SCREEN_FX_* */
    uint16_t effect_ms;     /* ms por caracter del efecto */
    uint8_t cursor;         /* SCREEN_CURSOR_BLOCK/NONE o caracter custom */
} screen_line_t;

typedef struct {
    screen_line_t line[SCREEN_LINES];
    uint16_t ms;            /* duracion; 0 = hasta que terminen las pasadas y
                             * efectos de una sola vez de sus lineas */
} screen_page_t;

typedef struct {
    uint8_t id;             /* 0..SCREEN_GLYPHS-1 */
    uint8_t n_frames;
    uint16_t frame_ms;      /* cada cuanto cambia de frame */
    const uint8_t *frames;  /* n_frames x 7 filas, bits 4..0 = columnas de
                             * izquierda a derecha */
} screen_glyph_t;

typedef struct {
    const screen_glyph_t *glyphs;
    uint8_t n_glyphs;
    const screen_page_t *pages;
    uint8_t n_pages;
} screen_show_t;

/* Llamar despues de vfd_begin(). Deja las lineas vacias. */
void screen_begin(void);

/* Llamar seguido desde el loop principal, junto con vfd_poll(). */
void screen_poll(void);

/* Reemplaza los caracteres custom por los del show y reproduce sus paginas
 * en loop. Devuelve 0 si los frames distintos no entran en las 10
 * ubicaciones del display (y no cambia nada). */
uint8_t screen_play(const screen_show_t *show);

/* Deja de pasar de pagina; lo que se ve queda como esta. */
void screen_stop(void);

/* Cambia una linea (0 = arriba). Devuelve 0 si algun parametro es invalido.
 * Si hay un show en curso, la proxima pagina la vuelve a cambiar. */
uint8_t screen_set_line(uint8_t line, const screen_line_t *cfg);

/* Define (n_frames > 0) o borra (0) el caracter custom id. Los frames
 * iguales, de este o de otros caracteres, comparten ubicacion. Devuelve 0
 * si no entran en las 10 ubicaciones del display (y no cambia nada). */
uint8_t screen_set_glyph(uint8_t id, uint8_t n_frames, const uint8_t *frames, uint16_t frame_ms);

/* Ubicaciones del display usadas por los caracteres custom (0-10). */
uint8_t screen_slots_used(void);

/* Pagina que se esta mostrando. */
uint8_t screen_page(void);

#ifdef __cplusplus
}
#endif

#endif
