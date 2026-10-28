/*
 * doom_live_test.c - Pasa una serie de estados del juego, en el mismo texto
 * que manda el parche de Chocolate Doom, a doom_live.c con un reloj
 * simulado. check_doom_live.py verifica lo que queda en el display.
 */
#include <stdio.h>
#include <stdint.h>
#include "vfd.h"
#include "screen.h"
#include "doom_live.h"

extern uint16_t host_ms;

static const char *const steps[] = {
    "ammo=50&health=100&armor=0&keys=&face=1",
    "ammo=-1&keys=b",                       /* puño: sin municion */
    "ammo=48&health=83&face=13",            /* ouch, dolor 1 */
    "health=61&armor=200&keys=br&face=22",  /* sonrisa malvada, dolor 2 */
    "face=40",                              /* modo dios */
    "health=-5&face=41",                    /* muerto */
};

int main(void)
{
    doom_status_t s = { 0, 0, 0, 0, 0 };
    uint8_t i;

    vfd_begin();
    screen_begin();
    for (host_ms = 0; host_ms < 1000; host_ms++) {   /* arranque del display */
        vfd_poll();
        screen_poll();
    }
    doom_live_begin();
    for (i = 0; i < sizeof steps / sizeof steps[0]; i++) {
        uint16_t end = (uint16_t)(host_ms + 500);
        doom_live_parse(steps[i], &s);
        doom_live_update(&s);
        for (; host_ms < end; host_ms++) {
            vfd_poll();
            screen_poll();
        }
    }
    return 0;
}
