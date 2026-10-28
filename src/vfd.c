/*
 * vfd.c - Driver portable y no bloqueante del VFD IEE 03601-95B-40. El
 * acceso al hardware pasa por vfd_hal.h.
 */
#include "vfd.h"
#include "vfd_hal.h"

#define RESET_MS    2       /* RESET bajo > 50 us (2 ticks = al menos 1 ms) */
#define BOOT_MS     551     /* reset por hardware: 500 ms max */

enum { ST_RESET, ST_BOOT, ST_SETUP, ST_READY };

static uint8_t state;
static uint16_t t_mark;
static uint8_t setup_pos;

static uint8_t queue[VFD_QUEUE_SIZE];
static uint8_t q_head;
static uint8_t q_len;
static uint8_t line_start;        /* primer caracter de la ultima vfd_line() */

/* ---- Inicializacion ----------------------------------------------------- */

/* El reset por software deja el display en su estado inicial aunque la
 * linea RESET no este conectada. */
static const uint8_t setup_cmds[] = {
    VFD_SOFT_RESET, VFD_CURSOR_OFF, VFD_MODE_WRAP, VFD_BRIGHTEST, VFD_CLEAR
};

#define SETUP_LEN sizeof setup_cmds

void vfd_begin(void)
{
    vfd_hal_init();
    vfd_hal_reset(1);
    t_mark = vfd_hal_millis();
    state = ST_RESET;
    q_len = 0;
}

/* ---- Envio -------------------------------------------------------------- */

static uint8_t send(uint8_t b)
{
    if (vfd_hal_busy())
        return 0;
    vfd_hal_wait_20us();
    vfd_hal_strobe(b);
    return 1;
}

void vfd_poll(void)
{
    uint16_t elapsed = (uint16_t)(vfd_hal_millis() - t_mark);

    switch (state) {
    case ST_RESET:
        if (elapsed >= RESET_MS) {
            vfd_hal_reset(0);
            t_mark += elapsed;
            state = ST_BOOT;
        }
        break;
    case ST_BOOT:
        if (elapsed >= BOOT_MS) {
            setup_pos = 0;
            state = ST_SETUP;
        }
        break;
    case ST_SETUP:
        if (send(setup_cmds[setup_pos]) && ++setup_pos == SETUP_LEN)
            state = ST_READY;
        break;
    default:
        if (q_len && send(queue[q_head])) {
            if (++q_head == VFD_QUEUE_SIZE)
                q_head = 0;
            q_len--;
        }
        break;
    }
}

uint8_t vfd_idle(void)
{
    return state == ST_READY && q_len == 0;
}

/* ---- Cola --------------------------------------------------------------- */

uint8_t vfd_room(void)
{
    return VFD_QUEUE_SIZE - q_len;
}

uint8_t vfd_write(uint8_t b)
{
    uint8_t tail;

    if (q_len == VFD_QUEUE_SIZE)
        return 0;
    tail = q_head + q_len;
    if (tail >= VFD_QUEUE_SIZE)
        tail -= VFD_QUEUE_SIZE;
    queue[tail] = b;
    q_len++;
    return 1;
}

uint8_t vfd_goto(uint8_t pos)
{
    if (vfd_room() < 2)
        return 0;
    vfd_write(VFD_GOTO);
    return vfd_write(pos);
}

uint8_t vfd_line(uint8_t pos)
{
    uint8_t i;

    if (vfd_room() < 2 + VFD_COLS)
        return 0;
    vfd_goto(pos);
    line_start = q_head + q_len;
    if (line_start >= VFD_QUEUE_SIZE)
        line_start -= VFD_QUEUE_SIZE;
    for (i = 0; i < VFD_COLS; i++)
        vfd_write(' ');
    return 1;
}

void vfd_line_put(uint8_t col, uint8_t c)
{
    uint8_t i = line_start + col;
    if (i >= VFD_QUEUE_SIZE)
        i -= VFD_QUEUE_SIZE;
    queue[i] = c;
}

uint8_t vfd_puts_at(uint8_t pos, const char *s)
{
    uint8_t len = 0;

    while (s[len])
        len++;
    if (vfd_room() < 2 + len)
        return 0;
    vfd_goto(pos);
    while (*s)
        vfd_write((uint8_t)*s++);
    return 1;
}

uint8_t vfd_clear(void)
{
    return vfd_write(VFD_CLEAR);
}
