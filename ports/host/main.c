/*
 * main.c - Corre un show de la libreria en la PC con un reloj simulado que
 * avanza 1 ms por vuelta del loop, unos dos ciclos: la demo, o la barra del
 * Doom con "vfd_host doom".
 */
#include <stdint.h>
#include <string.h>
#include "vfd.h"
#include "screen.h"
#include "demo.h"
#include "doom.h"

#define RUN_MS 32000u

extern uint16_t host_ms;

int main(int argc, char **argv)
{
    vfd_begin();
    screen_begin();
    screen_play(argc > 1 && strcmp(argv[1], "doom") == 0 ? &doom_show : &demo_show);
    for (host_ms = 0; host_ms < RUN_MS; host_ms++) {
        vfd_poll();
        screen_poll();
    }
    return 0;
}
