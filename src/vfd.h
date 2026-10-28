/*
 * vfd.h - Driver portable y no bloqueante del VFD IEE 03601-95B-40
 * (2 lineas x 20).
 *
 * vfd_write() y las funciones de texto solo encolan bytes; vfd_poll() los
 * manda al display de a uno, cuando no esta ocupado. Hay que llamar a
 * vfd_poll() seguido (cada pocos ms) desde el loop principal.
 */
#ifndef VFD_H
#define VFD_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define VFD_COLS 20
#define VFD_LINE2 VFD_COLS              /* posicion de inicio de la linea inferior */

#ifndef VFD_QUEUE_SIZE
#define VFD_QUEUE_SIZE 40               /* bytes pendientes de mandar */
#endif

/* Comandos (hoja de datos sec. 4.2) */
#define VFD_CURSOR_OFF  0x0E
#define VFD_MODE_WRAP   0x11
#define VFD_SOFT_RESET  0x14
#define VFD_CLEAR       0x15
#define VFD_LOAD_UDC    0x18
#define VFD_GOTO        0x1B
#define VFD_BRIGHTEST   0x1F

/* Arranca el reset y la configuracion; los completa vfd_poll() (unos
 * 600 ms). */
void vfd_begin(void);

/* Avanza el driver: manda como mucho un byte por llamada. */
void vfd_poll(void);

/* 1 cuando termino la inicializacion y no hay nada pendiente. */
uint8_t vfd_idle(void);

/* Lugar libre en la cola. */
uint8_t vfd_room(void);

/* Encolan bytes. Devuelven 0 si no habia lugar (y no encolan nada). */
uint8_t vfd_write(uint8_t b);
uint8_t vfd_goto(uint8_t pos);
uint8_t vfd_puts_at(uint8_t pos, const char *s);
uint8_t vfd_clear(void);

/* Encola una linea entera: el GOTO a pos y VFD_COLS espacios. Despues,
 * antes de llamar a vfd_poll(), vfd_line_put() cambia cualquiera de esos
 * caracteres. Asi se arma una linea sin un buffer aparte. Devuelve 0 si no
 * hay lugar. */
uint8_t vfd_line(uint8_t pos);
void vfd_line_put(uint8_t col, uint8_t c);

/* Caracteres custom: 10 ubicaciones, que se escriben con los codigos
 * VFD_UDC_FIRST..0xFF. rows[r], bits 4..0 = columnas de izquierda a derecha
 * (mismo formato que LiquidCrystal::createChar). Encola 8 bytes; devuelve 0
 * si no hay lugar o slot >= VFD_UDC_SLOTS. Esta en vfd_char.c. */
#define VFD_UDC_SLOTS 10
#define VFD_UDC_FIRST 0xF6
uint8_t vfd_define_char(uint8_t slot, const uint8_t rows[7]);

#ifdef __cplusplus
}
#endif

#endif
