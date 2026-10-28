/*
 * screen.c - Pantallas de mensajes, portable y no bloqueante, sobre vfd.h.
 *
 * El texto se lee del puntero del usuario cada vez que se dibuja (no se
 * copia), asi entra en la RAM interna de un 8051. Cada linea se reescribe
 * entera (22 bytes) cuando cambia: un paso de desplazamiento o de efecto,
 * el titileo del cursor o el frame de un caracter custom que contiene.
 *
 * Los frames de los caracteres custom se reparten entre las 10 ubicaciones
 * del display; los que son iguales (aunque sean de distintos caracteres)
 * comparten ubicacion.
 */
#include "screen.h"
#include "vfd.h"
#include "vfd_hal.h"
#define CHARSET_DEFINE_DATA
#include "charset.h"

/* En el 8051 la RAM directa no alcanza: el estado va a la RAM indirecta */
#ifdef __SDCC_mcs51
#define SCREEN_RAM __idata
#else
#define SCREEN_RAM
#endif

#define GLYPH_MARK  0x80            /* 0x80 | id: caracter custom en el texto */
#define LINE_BYTES  (2 + VFD_COLS)  /* goto + 20 caracteres */
#define BLOCK_CODE  0xEF            /* rectangulo lleno en la fuente del display */
#define END         0x00

/* Fases de los efectos */
enum { PH_TYPING, PH_FULL, PH_ERASING, PH_EMPTY };

typedef struct {
    const char *text;
    uint8_t len;
    uint8_t align;
    uint8_t scroll;
    uint8_t effect;
    uint8_t cursor;
    uint16_t ms;                /* ms por paso: del desplazamiento o del efecto
                                 * (los efectos son solo para lineas fijas) */
    uint16_t t_step;
    uint8_t step;               /* posicion de la marquesina o de la pasada */
    uint8_t phase;              /* efecto */
    uint8_t pos;                /* caracteres tipeados o borrados */
    uint8_t blink;
    uint16_t t_blink;
    uint16_t glyph_mask;        /* bit n: el texto usa el caracter custom n */
    uint8_t dirty;
} line_t;

typedef struct {
    const uint8_t *frames;
    uint8_t n_frames;
    uint8_t frame;
    uint8_t slot;               /* ubicacion del frame actual */
    uint16_t frame_ms;
    uint16_t t_step;
} glyph_t;

static SCREEN_RAM line_t lines[SCREEN_LINES];
static SCREEN_RAM glyph_t glyphs[SCREEN_GLYPHS];
static const uint8_t *SCREEN_RAM slot_rows[VFD_UDC_SLOTS];
static uint8_t slots_used;
static uint16_t slots_pending;      /* bit s: falta cargar la ubicacion s */

static const screen_show_t *show;
static uint8_t page;
static uint16_t t_page;

/* ---- Texto -------------------------------------------------------------- */

/* Caracter Unicode que no es ASCII -> codigo del display (charset.h, sacado
 * de la tabla de caracteres de la hoja de datos; no es Latin-1). */
static uint8_t unicode_code(uint16_t u)
{
    uint8_t i;
    for (i = 0; i < CHARSET_LEN; i++)
        if (charset[i].unicode == u)
            return charset[i].code;
    return '?';
}

/* Lee el proximo caracter de *s y lo devuelve como codigo del display (o
 * GLYPH_MARK | id). END al final del texto. */
static uint8_t next_char(const char **s)
{
    const uint8_t *p = (const uint8_t *)*s;
    uint8_t c = *p++;
    uint16_t u;

    if (!c)
        return END;
    if (c == '{' && *p == '{') {
        p++;
    } else if (c == '{' && p[0] >= '0' && p[0] <= '9' && p[1] == '}') {
        c = (uint8_t)(GLYPH_MARK | (p[0] - '0'));
        p += 2;
    } else if (c >= 0x80) {
        /* UTF-8 de 2 o 3 bytes; lo demas no esta en el display */
        if ((c & 0xE0) == 0xC0 && (p[0] & 0xC0) == 0x80) {
            u = (uint16_t)(((c & 0x1F) << 6) | (p[0] & 0x3F));
            p += 1;
            c = unicode_code(u);
        } else if ((c & 0xF0) == 0xE0 && (p[0] & 0xC0) == 0x80 && (p[1] & 0xC0) == 0x80) {
            u = (uint16_t)(((c & 0x0F) << 12) | ((p[0] & 0x3F) << 6) | (p[1] & 0x3F));
            p += 2;
            c = unicode_code(u);
        } else {
            while ((*p & 0xC0) == 0x80)
                p++;
            c = '?';
        }
    } else if (c < 0x20 || c == 0x7F) {
        c = ' ';
    }
    *s = (const char *)p;
    return c;
}

/* Cuenta los caracteres de la linea y anota que caracteres custom usa */
static void scan_text(SCREEN_RAM line_t *l)
{
    const char *s = l->text;
    uint8_t c;
    l->len = 0;
    l->glyph_mask = 0;
    while (l->len < SCREEN_TEXT_MAX && (c = next_char(&s)) != END) {
        if ((c & GLYPH_MARK) && c < 0xA0)
            l->glyph_mask |= (uint16_t)(1u << (c & 0x7F));
        l->len++;
    }
}

/* ---- Caracteres custom -------------------------------------------------- */

static uint8_t same_rows(const uint8_t *a, const uint8_t *b)
{
    uint8_t r;
    for (r = 0; r < 7; r++)
        if ((a[r] ^ b[r]) & 0x1F)
            return 0;
    return 1;
}

static uint8_t find_slot(const uint8_t *rows)
{
    uint8_t s;
    for (s = 0; s < slots_used; s++)    /* casi siempre es el mismo puntero */
        if (slot_rows[s] == rows)
            return s;
    for (s = 0; s < slots_used; s++)
        if (same_rows(slot_rows[s], rows))
            return s;
    return 0;
}

/* Reparte los frames de todos los caracteres entre las ubicaciones del
 * display. Devuelve 0 si no alcanzan. */
static uint8_t allocate(void)
{
    uint8_t i, f, s;
    slots_used = 0;
    for (i = 0; i < SCREEN_GLYPHS; i++) {
        SCREEN_RAM glyph_t *g = &glyphs[i];
        for (f = 0; f < g->n_frames; f++) {
            const uint8_t *rows = g->frames + f * 7;
            for (s = 0; s < slots_used && !same_rows(slot_rows[s], rows); s++)
                ;
            if (s == slots_used) {
                if (slots_used == VFD_UDC_SLOTS)
                    return 0;
                slot_rows[slots_used++] = rows;
            }
        }
    }
    slots_pending = (uint16_t)((1u << slots_used) - 1);
    for (i = 0; i < SCREEN_GLYPHS; i++) {
        SCREEN_RAM glyph_t *g = &glyphs[i];
        if (g->n_frames)
            g->slot = find_slot(g->frames + g->frame * 7);
    }
    for (i = 0; i < SCREEN_LINES; i++)
        lines[i].dirty = 1;     /* los codigos pueden haber cambiado */
    return 1;
}

static void glyph_init(SCREEN_RAM glyph_t *g, uint8_t n_frames, const uint8_t *frames, uint16_t frame_ms)
{
    g->frames = frames;
    g->n_frames = n_frames;
    g->frame = 0;
    g->frame_ms = frame_ms;
    g->t_step = vfd_hal_millis();
}

/* ---- Lineas ------------------------------------------------------------- */

static uint8_t is_pass(const SCREEN_RAM line_t *l)
{
    return l->scroll == SCREEN_PASS_LEFT || l->scroll == SCREEN_PASS_RIGHT;
}

/* Vuelta completa de la marquesina: el texto y un hueco */
static uint8_t period(const SCREEN_RAM line_t *l)
{
    uint8_t gap = l->len < VFD_COLS - SCREEN_GAP ? VFD_COLS - l->len : SCREEN_GAP;
    return (uint8_t)(l->len + gap);
}

static uint8_t fx_active(const SCREEN_RAM line_t *l)
{
    return l->effect != SCREEN_FX_NONE && l->scroll == SCREEN_STATIC;
}

static uint8_t cursor_visible(const SCREEN_RAM line_t *l)
{
    return fx_active(l) && l->cursor != SCREEN_CURSOR_NONE &&
           (l->phase == PH_TYPING || l->phase == PH_ERASING);
}

static uint8_t line_has_glyph(const SCREEN_RAM line_t *l, uint8_t id)
{
    return (cursor_visible(l) && l->cursor == id) || (l->glyph_mask >> id) & 1;
}

/* La linea no va a cambiar mas sola (para las paginas de duracion 0) */
static uint8_t line_finished(const SCREEN_RAM line_t *l)
{
    if (l->dirty)
        return 0;
    if (is_pass(l))
        return l->step >= VFD_COLS + l->len;
    if (l->scroll != SCREEN_STATIC)
        return 0;
    switch (l->effect) {
    case SCREEN_FX_TYPE:
        return l->phase == PH_FULL;
    case SCREEN_FX_ERASE:
        return l->phase == PH_EMPTY;
    case SCREEN_FX_CYCLE:
        return 0;
    default:
        return 1;
    }
}

static void line_restart(SCREEN_RAM line_t *l)
{
    uint16_t now = vfd_hal_millis();
    l->step = 0;
    l->t_step = now;
    l->phase = l->effect == SCREEN_FX_ERASE ? PH_ERASING : PH_TYPING;
    l->pos = 0;
    l->blink = 1;
    l->t_blink = now;
    l->dirty = 1;
}

static uint8_t valid_cursor(uint8_t c)
{
    return c < SCREEN_GLYPHS || c == SCREEN_CURSOR_BLOCK || c == SCREEN_CURSOR_NONE;
}

uint8_t screen_set_line(uint8_t line, const screen_line_t *cfg)
{
    SCREEN_RAM line_t *l;
    if (line >= SCREEN_LINES || !cfg->text || cfg->align > SCREEN_RIGHT ||
        cfg->scroll > SCREEN_PASS_RIGHT || (cfg->scroll && !cfg->scroll_ms) ||
        cfg->effect > SCREEN_FX_CYCLE || (cfg->effect && !cfg->effect_ms) ||
        (cfg->effect && !valid_cursor(cfg->cursor)))
        return 0;
    l = &lines[line];
    l->text = cfg->text;
    scan_text(l);
    l->align = cfg->align;
    l->scroll = cfg->scroll;
    l->effect = cfg->effect;
    l->cursor = cfg->cursor;
    l->ms = cfg->scroll ? cfg->scroll_ms : cfg->effect_ms;
    line_restart(l);
    return 1;
}

/* Avanza el efecto un paso. Devuelve 1 si cambio lo que se ve. */
static uint8_t fx_step(SCREEN_RAM line_t *l, uint16_t now)
{
    uint16_t wait;

    if ((l->effect == SCREEN_FX_TYPE && l->phase == PH_FULL) ||
        (l->effect == SCREEN_FX_ERASE && l->phase == PH_EMPTY))
        return 0;               /* efecto de una sola vez, terminado */
    wait = l->phase == PH_FULL ? SCREEN_HOLD_MS : l->phase == PH_EMPTY ? SCREEN_GAP_MS : l->ms;
    if ((uint16_t)(now - l->t_step) < wait)
        return 0;
    l->t_step = now;
    switch (l->phase) {
    case PH_TYPING:
        if (l->pos < l->len)
            l->pos++;
        else
            l->phase = PH_FULL;
        break;
    case PH_FULL:
        l->phase = PH_ERASING;
        l->pos = 0;
        break;
    case PH_ERASING:
        if (l->pos < l->len)
            l->pos++;
        else
            l->phase = PH_EMPTY;
        break;
    default:
        l->phase = PH_TYPING;
        l->pos = 0;
        break;
    }
    return 1;
}

/* Avanza la marquesina o la pasada un paso. Devuelve 1 si cambio. */
static uint8_t scroll_step(SCREEN_RAM line_t *l, uint16_t now)
{
    if (l->scroll == SCREEN_STATIC || (uint16_t)(now - l->t_step) < l->ms)
        return 0;
    if (is_pass(l) && l->step >= VFD_COLS + l->len)
        return 0;               /* la pasada termino */
    l->t_step = now;
    l->step++;
    if (!is_pass(l) && l->step >= period(l))
        l->step = 0;
    return 1;
}

/* ---- Dibujo ------------------------------------------------------------- */

static uint8_t code(uint8_t c)
{
    const SCREEN_RAM glyph_t *g;
    if (!(c & GLYPH_MARK) || c >= 0xA0)
        return c;
    g = &glyphs[c & 0x7F];
    return g->n_frames ? (uint8_t)(VFD_UDC_FIRST + g->slot) : ' ';
}

/* Columna donde va el caracter k (fuera de 0..VFD_COLS-1 si no se ve) */
static int16_t column(const SCREEN_RAM line_t *l, uint8_t k, int16_t start)
{
    int16_t p;
    switch (l->scroll) {
    case SCREEN_SCROLL_LEFT:
        p = period(l);
        return (int16_t)((k - l->step + p) % p);
    case SCREEN_SCROLL_RIGHT:
        p = period(l);
        return (int16_t)((k + l->step) % p);
    case SCREEN_PASS_LEFT:
        return (int16_t)(VFD_COLS - l->step + k);
    case SCREEN_PASS_RIGHT:
        return (int16_t)(l->step - l->len + k);
    default:
        return (int16_t)(start + k);
    }
}

static void render(uint8_t li)
{
    const SCREEN_RAM line_t *l = &lines[li];
    const char *s = l->text;
    uint8_t k, c;
    int16_t start = 0, col;

    vfd_line((uint8_t)(li * VFD_COLS));     /* screen_poll() ya vio que hay lugar */
    if (l->len < VFD_COLS) {
        if (l->align == SCREEN_CENTER)
            start = (VFD_COLS - l->len) / 2;
        else if (l->align == SCREEN_RIGHT)
            start = VFD_COLS - l->len;
    }
    for (k = 0; k < l->len && (c = next_char(&s)) != END; k++) {
        if (fx_active(l) && (l->phase == PH_EMPTY ||
                             (l->phase == PH_TYPING && k >= l->pos) ||
                             (l->phase == PH_ERASING && k < l->pos)))
            continue;
        col = column(l, k, start);
        if (col >= 0 && col < VFD_COLS)
            vfd_line_put((uint8_t)col, code(c));
    }
    if (cursor_visible(l) && start + l->pos < VFD_COLS) {
        if (l->cursor == SCREEN_CURSOR_BLOCK)
            c = l->blink ? BLOCK_CODE : ' ';
        else
            c = code((uint8_t)(GLYPH_MARK | l->cursor));
        vfd_line_put((uint8_t)(start + l->pos), c);
    }
}

/* ---- Shows -------------------------------------------------------------- */

static void show_page(void)
{
    uint8_t i;
    const screen_page_t *p = &show->pages[page];
    for (i = 0; i < SCREEN_LINES; i++)
        if (p->line[i].text)
            screen_set_line(i, &p->line[i]);
    t_page = vfd_hal_millis();
}

static uint8_t page_done(uint16_t now)
{
    const screen_page_t *p = &show->pages[page];
    uint8_t i;
    if (slots_pending)
        return 0;
    if (p->ms)
        return (uint16_t)(now - t_page) >= p->ms;
    for (i = 0; i < SCREEN_LINES; i++)
        if (!line_finished(&lines[i]))
            return 0;
    return 1;
}

uint8_t screen_play(const screen_show_t *s)
{
    uint8_t i;
    for (i = 0; i < SCREEN_GLYPHS; i++)
        glyphs[i].n_frames = 0;
    for (i = 0; i < s->n_glyphs; i++) {
        const screen_glyph_t *d = &s->glyphs[i];
        if (d->id < SCREEN_GLYPHS)
            glyph_init(&glyphs[d->id], d->n_frames, d->frames, d->frame_ms);
    }
    if (!allocate()) {
        for (i = 0; i < SCREEN_GLYPHS; i++)
            glyphs[i].n_frames = 0;
        allocate();
        show = 0;
        return 0;
    }
    show = s;
    page = 0;
    if (s->n_pages)
        show_page();
    return 1;
}

void screen_stop(void)
{
    show = 0;
}

uint8_t screen_page(void)
{
    return page;
}

/* ---- API ---------------------------------------------------------------- */

void screen_begin(void)
{
    static const screen_line_t blank = { "", SCREEN_LEFT, SCREEN_STATIC, 0, SCREEN_FX_NONE, 0, 0 };
    uint8_t i;
    for (i = 0; i < SCREEN_GLYPHS; i++)
        glyphs[i].n_frames = 0;
    slots_used = 0;
    slots_pending = 0;
    show = 0;
    for (i = 0; i < SCREEN_LINES; i++)
        screen_set_line(i, &blank);
}

uint8_t screen_slots_used(void)
{
    return slots_used;
}

uint8_t screen_set_glyph(uint8_t id, uint8_t n_frames, const uint8_t *frames, uint16_t frame_ms)
{
    const uint8_t *old_frames;
    uint8_t old_n;
    uint16_t old_ms;

    if (id >= SCREEN_GLYPHS)
        return 0;
    old_frames = glyphs[id].frames;
    old_n = glyphs[id].n_frames;
    old_ms = glyphs[id].frame_ms;
    glyph_init(&glyphs[id], n_frames, frames, frame_ms);
    if (!allocate()) {
        glyph_init(&glyphs[id], old_n, old_frames, old_ms);
        allocate();
        return 0;
    }
    return 1;
}

void screen_poll(void)
{
    uint16_t now = vfd_hal_millis();
    uint8_t i, j;

    for (i = 0; i < SCREEN_LINES; i++) {
        SCREEN_RAM line_t *l = &lines[i];
        if (scroll_step(l, now))
            l->dirty = 1;
        if (fx_active(l) && fx_step(l, now))
            l->dirty = 1;
        if (cursor_visible(l) && l->cursor == SCREEN_CURSOR_BLOCK &&
            (uint16_t)(now - l->t_blink) >= SCREEN_BLINK_MS) {
            l->t_blink = now;
            l->blink = !l->blink;
            l->dirty = 1;
        }
    }
    /* Todos los cambios de frame que vencen juntos van en la misma llamada:
     * asi un dibujo de varios caracteres (como la cara de doom.c) cambia
     * entero, sin mitades desparejas */
    for (i = 0; i < SCREEN_GLYPHS; i++) {
        SCREEN_RAM glyph_t *g = &glyphs[i];
        if (g->n_frames > 1 && (uint16_t)(now - g->t_step) >= g->frame_ms) {
            uint8_t old = g->slot;
            g->t_step = now;
            g->frame = (uint8_t)((g->frame + 1) % g->n_frames);
            g->slot = find_slot(g->frames + g->frame * 7);
            if (g->slot != old)
                for (j = 0; j < SCREEN_LINES; j++)
                    if (line_has_glyph(&lines[j], i))
                        lines[j].dirty = 1;
        }
    }
    if (show && show->n_pages > 1 && page_done(now)) {
        page = (uint8_t)((page + 1) % show->n_pages);
        show_page();
    }

    /* Primero los caracteres custom pendientes, despues las lineas */
    if (slots_pending) {
        for (i = 0; i < slots_used; i++) {
            if (slots_pending & (1u << i)) {
                if (vfd_define_char(i, slot_rows[i]))
                    slots_pending &= (uint16_t)~(1u << i);
                return;
            }
        }
    }
    for (i = 0; i < SCREEN_LINES; i++) {
        if (lines[i].dirty && vfd_room() >= LINE_BYTES) {
            render(i);
            lines[i].dirty = 0;
            return;
        }
    }
}
