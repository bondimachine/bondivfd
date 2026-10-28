/*
 * hal_arduino.cpp - HAL del VFD para cualquier placa Arduino. Usa solo la
 * API estandar (pinMode, digitalWrite, digitalRead, millis,
 * delayMicroseconds), asi que no depende del microcontrolador.
 */
#ifdef ARDUINO

#include <Arduino.h>
#include "BondiVFD.h"
#include "vfd_hal.h"

/* Tiempo maximo de ejecucion de un byte (sec. 4.7: el mayor es el reset por
 * software, 710 us) mas los 20 us previos al strobe. Pasado este tiempo el
 * display esta libre aunque BUSY no haya bajado (o no este conectado). */
#define MAX_EXEC_US 750

static vfd_pins_t pins = VFD_DEFAULT_PINS;
static uint8_t data_pins[8];    /* D0..D7, para recorrerlos en orden */
static uint32_t last_strobe_us;

extern "C" {

void vfd_set_pins(const vfd_pins_t *p)
{
    pins = *p;
}

void vfd_hal_init(void)
{
    data_pins[0] = pins.d0;
    data_pins[1] = pins.d1;
    data_pins[2] = pins.d2;
    data_pins[3] = pins.d3;
    data_pins[4] = pins.d4;
    data_pins[5] = pins.d5;
    data_pins[6] = pins.d6;
    data_pins[7] = pins.d7;
    for (uint8_t i = 0; i < 8; i++) {
        pinMode(data_pins[i], OUTPUT);
        digitalWrite(data_pins[i], LOW);
    }
    digitalWrite(pins.wr, LOW);
    pinMode(pins.wr, OUTPUT);
    if (pins.reset != VFD_NO_PIN) {
        digitalWrite(pins.reset, HIGH);
        pinMode(pins.reset, OUTPUT);
    }
    if (pins.busy != VFD_NO_PIN)
        pinMode(pins.busy, INPUT);
    last_strobe_us = micros() - MAX_EXEC_US;
}

void vfd_hal_reset(uint8_t asserted)
{
    if (pins.reset != VFD_NO_PIN)
        digitalWrite(pins.reset, asserted ? LOW : HIGH);
}

uint8_t vfd_hal_busy(void)
{
    if ((uint32_t)(micros() - last_strobe_us) >= MAX_EXEC_US)
        return 0;
    if (pins.busy == VFD_NO_PIN)
        return 1;
    return digitalRead(pins.busy) == HIGH;
}

/* Las esperas de 1 us cubren el pulso minimo de 250 ns y los 500 ns que
 * tarda BUSY en subir aunque la placa sea muy rapida (ESP32, RP2040...). */
void vfd_hal_strobe(uint8_t b)
{
    for (uint8_t i = 0; i < 8; i++)
        digitalWrite(data_pins[i], (b >> i) & 1 ? HIGH : LOW);
    digitalWrite(pins.wr, HIGH);
    last_strobe_us = micros();
    delayMicroseconds(1);
    digitalWrite(pins.wr, LOW);
    delayMicroseconds(1);
}

void vfd_hal_wait_20us(void)
{
    delayMicroseconds(25);
}

uint16_t vfd_hal_millis(void)
{
    return (uint16_t)millis();
}

}  /* extern "C" */

#endif  /* ARDUINO */
