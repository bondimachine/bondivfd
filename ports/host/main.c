/*
 * main.c - Corre la demo (src/demo.c) en la PC con un reloj simulado que
 * avanza 1 ms por vuelta del loop, unos dos ciclos.
 */
#include <stdint.h>
#include "vfd.h"
#include "screen.h"
#include "demo.h"

#define RUN_MS 32000u

extern uint16_t host_ms;

int main(void)
{
    vfd_begin();
    screen_begin();
    screen_play(&demo_show);
    for (host_ms = 0; host_ms < RUN_MS; host_ms++) {
        vfd_poll();
        screen_poll();
    }
    return 0;
}
