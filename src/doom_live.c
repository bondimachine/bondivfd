/*
 * doom_live.c - Barra de estado del Doom en vivo (ver doom_live.h).
 *
 * La cara se elige como en st_stuff.c de Doom: 8 caras por nivel de dolor
 * (de frente mirando a 3 lados, girando a la derecha y a la izquierda,
 * ouch, sonrisa malvada y furia), despues la del modo dios (40) y la de
 * muerto (41). Los dibujos estan en doom_faces.py.
 */
#include "doom_live.h"
#include "screen.h"
#include "doom_faces.h"

#define FACE_GOD    40
#define FACE_DEAD   41

enum { EYES_LEFT, EYES_CENTER, EYES_RIGHT, EYES_WIDE };
enum { MOUTH_GRIMACE, MOUTH_GRIN, MOUTH_OUCH, MOUTH_DEAD };

/* Ojos y boca de cada una de las 8 caras de un nivel de dolor */
static const uint8_t face_kind[8][2] = {
    { EYES_LEFT, MOUTH_GRIMACE }, { EYES_CENTER, MOUTH_GRIMACE }, { EYES_RIGHT, MOUTH_GRIMACE },
    { EYES_RIGHT, MOUTH_GRIMACE },          /* girando a la derecha */
    { EYES_LEFT, MOUTH_GRIMACE },           /* girando a la izquierda */
    { EYES_WIDE, MOUTH_OUCH },              /* ouch */
    { EYES_CENTER, MOUTH_GRIN },            /* sonrisa malvada */
    { EYES_WIDE, MOUTH_GRIMACE },           /* furia */
};

static const uint8_t key_rows[7] = { 0x0E, 0x11, 0x0E, 0x04, 0x06, 0x04, 0x06 };

static char top[64];            /* screen.c guarda los punteros a los textos */
static char bottom[64];
static uint8_t face = 0xFF;

static char *put(char *p, const char *s)
{
    while (*s)
        *p++ = *s++;
    return p;
}

/* Numero alineado a la derecha en width caracteres (0..999) */
static char *num(char *p, int16_t v, uint8_t width)
{
    char digits[4];
    uint8_t n = 0;
    if (v < 0)
        v = 0;
    if (v > 999)
        v = 999;
    do {
        digits[n++] = (char)('0' + v % 10);
        v /= 10;
    } while (v);
    while (width-- > n)
        *p++ = ' ';
    while (n)
        *p++ = digits[--n];
    return p;
}

static void set_face(uint8_t index)
{
    const uint8_t *tl, *tr, *bl, *br;
    uint8_t pain;

    if (index > FACE_DEAD)
        index = FACE_DEAD;
    if (index == FACE_GOD || index == FACE_DEAD) {
        uint8_t k = index == FACE_DEAD;
        pain = k ? 4 : 0;
        tl = doom_face_special[k][0];
        tr = doom_face_special[k][1];
        bl = doom_face_bottom[pain][k ? MOUTH_DEAD : MOUTH_GRIMACE][0];
        br = doom_face_bottom[pain][k ? MOUTH_DEAD : MOUTH_GRIMACE][1];
    } else {
        const uint8_t *kind = face_kind[index % 8];
        pain = index / 8;
        tl = doom_face_top[pain][kind[0]][0];
        tr = doom_face_top[pain][kind[0]][1];
        bl = doom_face_bottom[pain][kind[1]][0];
        br = doom_face_bottom[pain][kind[1]][1];
    }
    screen_set_glyph(0, 1, tl, 0);
    screen_set_glyph(1, 1, tr, 0);
    screen_set_glyph(2, 1, bl, 0);
    screen_set_glyph(3, 1, br, 0);
}

void doom_live_begin(void)
{
    uint8_t i;
    screen_stop();
    for (i = 0; i < SCREEN_GLYPHS; i++)
        screen_set_glyph(i, 0, 0, 0);
    screen_set_glyph(4, 1, key_rows, 0);
    face = 0xFF;
}

void doom_live_update(const doom_status_t *s)
{
    static const char letters[] = "BYR";
    screen_line_t l = { 0, SCREEN_LEFT, SCREEN_STATIC, 0, SCREEN_FX_NONE, 0, 0 };
    char *p;
    uint8_t k;

    if (s->face != face) {
        face = s->face;
        set_face(face);
    }

    /*  "  47  61% {0}{1} 100%{4} {4}" : municion, salud, cara, armadura, llaves */
    p = top;
    if (s->ammo == DOOM_NO_AMMO)
        p = put(p, "    ");
    else
        p = num(p, s->ammo, 4);
    *p++ = ' ';
    p = num(p, s->health, 3);
    p = put(p, "% {0}{1} ");
    p = num(p, s->armor, 3);
    *p++ = '%';
    for (k = 0; k < 3; k++)
        p = put(p, s->keys & (1 << k) ? "{4}" : " ");
    *p = 0;

    /* "AMMO HLTH {2}{3} ARM B R " : etiquetas, la boca y la letra de cada llave */
    p = put(bottom, "AMMO HLTH {2}{3} ARM ");
    for (k = 0; k < 3; k++)
        *p++ = s->keys & (1 << k) ? letters[k] : ' ';
    *p = 0;

    l.text = top;
    screen_set_line(0, &l);
    l.text = bottom;
    screen_set_line(1, &l);
}

/* ---- Lectura del texto -------------------------------------------------- */

static uint8_t same_key(const char *p, const char *key)
{
    while (*key)
        if (*p++ != *key++)
            return 0;
    return *p == '=';
}

static int16_t read_int(const char *p)
{
    int16_t v = 0, sign = 1;
    if (*p == '-') {
        sign = -1;
        p++;
    }
    while (*p >= '0' && *p <= '9')
        v = (int16_t)(v * 10 + (*p++ - '0'));
    return (int16_t)(v * sign);
}

uint8_t doom_live_parse(const char *text, doom_status_t *s)
{
    const char *p = text;
    uint8_t n = 0;

    while (*p) {
        const char *v = p;
        while (*v && *v != '=' && *v != '&')
            v++;
        if (*v == '=') {
            v++;
            if (same_key(p, "ammo")) {
                s->ammo = read_int(v);
                if (s->ammo < 0)
                    s->ammo = DOOM_NO_AMMO;
                n++;
            } else if (same_key(p, "health")) {
                s->health = read_int(v);
                n++;
            } else if (same_key(p, "armor")) {
                s->armor = read_int(v);
                n++;
            } else if (same_key(p, "face")) {
                s->face = (uint8_t)read_int(v);
                n++;
            } else if (same_key(p, "keys")) {
                s->keys = 0;
                for (; *v && *v != '&' && *v != ' ' && *v != '\n'; v++) {
                    if (*v == 'b' || *v == 'B') s->keys |= DOOM_KEY_BLUE;
                    if (*v == 'y' || *v == 'Y') s->keys |= DOOM_KEY_YELLOW;
                    if (*v == 'r' || *v == 'R') s->keys |= DOOM_KEY_RED;
                }
                n++;
            }
        }
        /* al proximo campo */
        while (*v && *v != '&' && *v != ' ' && *v != '\n')
            v++;
        while (*v == '&' || *v == ' ' || *v == '\n' || *v == '\r')
            v++;
        p = v;
    }
    return n;
}
