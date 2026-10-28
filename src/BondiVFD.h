/*
 * BondiVFD.h - Header de la libreria Arduino BondiVFD: VFD IEE 03601-95B-40
 * conectado directo a pines digitales, con pantallas animadas (screen.h).
 *
 * Conexion por defecto (J1 del display):
 *
 *                         Arduino     ESP32 DevKit (30 pines, WROOM-32)
 *   D0..D7 (J1-10..J1-3)  pines 2..9  GPIO 15, 2, 4, 16, 17, 5, 18, 19
 *   WRITE STROBE (J1-2)   pin 10      GPIO 21
 *   RESET (J1-14)         pin 11      GPIO 22
 *   BUSY (J1-1)           pin 12      GPIO 23
 *
 * Para otros pines, llamar a vfd_set_pins() antes de vfd_begin() (ver el
 * ejemplo Demo.ino).
 */
#ifndef BONDIVFD_H
#define BONDIVFD_H

#include <stdint.h>
#include "vfd.h"
#include "screen.h"
#include "demo.h"
#include "doom.h"
#include "doom_live.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Pin de Arduino conectado a cada senal del display */
typedef struct {
    uint8_t busy;               /* J1-1 */
    uint8_t wr;                 /* J1-2, WRITE STROBE */
    uint8_t d7;                 /* J1-3 */
    uint8_t d6;                 /* J1-4 */
    uint8_t d5;                 /* J1-5 */
    uint8_t d4;                 /* J1-6 */
    uint8_t d3;                 /* J1-7 */
    uint8_t d2;                 /* J1-8 */
    uint8_t d1;                 /* J1-9 */
    uint8_t d0;                 /* J1-10 */
    uint8_t reset;              /* J1-14 */
} vfd_pins_t;

/* Senal sin conectar. Sin reset, alcanza con el reset por software que manda
 * vfd_begin(). Con o sin busy, despues de cada byte nunca se espera mas que
 * el tiempo maximo de ejecucion de la hoja de datos (sec. 4.7). */
#define VFD_NO_PIN 0xFF

/* busy, wr, d7..d0, reset (mismo orden que el struct) */
#if defined(ARDUINO_ARCH_ESP32) && defined(CONFIG_IDF_TARGET_ESP32)
/* ESP32 clasico, columna derecha del DevKit de 30 pines de abajo hacia
 * arriba (D15, D2, D4, RX2, TX2, D5, D18, D19, D21, D22, D23), salteando
 * RX0/TX0. GPIO 16-17 son de la PSRAM en los modulos WROVER: ahi no sirve.
 * Otros ESP32 (S2, S3, C3...) tienen otros pines: usar vfd_set_pins(). */
#define VFD_DEFAULT_PINS { 23, 21, 19, 18, 5, 17, 16, 4, 2, 15, 22 }
#else
#define VFD_DEFAULT_PINS { 12, 10, 9, 8, 7, 6, 5, 4, 3, 2, 11 }
#endif

/* Cambia los pines. Llamar antes de vfd_begin(). */
void vfd_set_pins(const vfd_pins_t *pins);

#ifdef __cplusplus
}
#endif

#endif
