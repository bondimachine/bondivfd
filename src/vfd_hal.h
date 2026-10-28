/*
 * vfd_hal.h - Lo que cada plataforma tiene que implementar para manejar el
 * VFD IEE 03601-95B-40. Todo lo demas (vfd.c, screen.c...) es C portable.
 *
 * Implementaciones: src/hal_arduino.cpp (cualquier placa Arduino),
 * ports/host/hal_host.c (PC, para pruebas). El repo de la BondiMachine tiene
 * uno para el SAB 80C537.
 */
#ifndef VFD_HAL_H
#define VFD_HAL_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Configura pines/puertos y timers. Deja WRITE STROBE en bajo. */
void vfd_hal_init(void);

/* 1 = mantener el display en reset (linea J1-14 en bajo), 0 = liberarlo. */
void vfd_hal_reset(uint8_t asserted);

/* 1 mientras el display esta ocupado (BUSY, J1-1, en alto). */
uint8_t vfd_hal_busy(void);

/* Pone el byte en D0-D7 y da un pulso en alto de WRITE STROBE de al menos
 * 250 ns. Antes de volver tiene que pasar al menos 500 ns desde el flanco de
 * subida, que es lo que tarda BUSY en subir (hoja de datos sheet 12). */
void vfd_hal_strobe(uint8_t b);

/* Espera al menos 20 us (entre que BUSY baja y el siguiente strobe). Es la
 * unica espera activa del driver. */
void vfd_hal_wait_20us(void);

/* Milisegundos desde el arranque; puede dar la vuelta (se compara con resta). */
uint16_t vfd_hal_millis(void);

#ifdef __cplusplus
}
#endif

#endif
