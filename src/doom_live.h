/*
 * doom_live.h - Barra de estado del Doom en vivo: muestra el estado que
 * manda el juego (municion, salud, armadura, llaves y la cara) sin reiniciar
 * nada, para conectarla a un Doom de verdad (ver tools/chocolate-doom).
 *
 *   |  47  61% {0}{1} 100%{4} {4}|   {0}..{3}: la cara del juego
 *   |AMMO HLTH {2}{3} ARM B R    |   {4}: llave (con su letra abajo)
 *
 *   doom_live_begin();                    // una vez, despues de screen_begin()
 *   doom_live_update(&status);            // cada vez que cambia algo
 */
#ifndef DOOM_LIVE_H
#define DOOM_LIVE_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DOOM_KEY_BLUE   0x01
#define DOOM_KEY_YELLOW 0x02
#define DOOM_KEY_RED    0x04

#define DOOM_NO_AMMO    (-1)    /* arma sin municion (puño, motosierra) */

typedef struct {
    int16_t ammo;               /* municion del arma actual, o DOOM_NO_AMMO */
    int16_t health;             /* se muestra entre 0 y 999 */
    int16_t armor;
    uint8_t keys;               /* DOOM_KEY_*: tarjetas o calaveras */
    uint8_t face;               /* st_faceindex de Doom (st_stuff.c), 0-41 */
} doom_status_t;

/* Deja de reproducir cualquier show y arma la barra vacia. */
void doom_live_begin(void);

/* Muestra el estado. Solo reescribe lo que cambio: la linea de los numeros
 * y, si cambia la cara, sus 4 caracteres custom. */
void doom_live_update(const doom_status_t *s);

/* Lee un estado en texto "ammo=47&health=61&armor=100&keys=br&face=12"
 * (cualquier orden; tambien separado por espacios o saltos de linea). Los
 * campos que faltan quedan como estaban en s. Devuelve la cantidad de campos
 * leidos. */
uint8_t doom_live_parse(const char *text, doom_status_t *s);

#ifdef __cplusplus
}
#endif

#endif
