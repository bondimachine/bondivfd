/*
 * hal_host.c - HAL de prueba para PC: en vez de manejar un display, escribe
 * en stdout cada byte que recibiria el VFD (uno por linea, en hex, con los
 * ms adelante si se compila con HOST_TIMESTAMPS). tools/vfd_model.py y
 * check_screen.py la reproducen en el modelo del display.
 */
#include <stdio.h>
#include "vfd_hal.h"

uint16_t host_ms;               /* reloj simulado, lo avanza main.c */

void vfd_hal_init(void) {}
void vfd_hal_reset(uint8_t asserted) { (void)asserted; }
uint8_t vfd_hal_busy(void) { return 0; }
void vfd_hal_strobe(uint8_t b)
{
#ifdef HOST_TIMESTAMPS
    printf("%u ", host_ms);
#endif
    printf("%02X\n", b);
}
void vfd_hal_wait_20us(void) {}
uint16_t vfd_hal_millis(void) { return host_ms; }
