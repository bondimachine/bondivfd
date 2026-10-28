# BondiVFD

Librería para el VFD **IEE 03601-95B-40** (2 líneas × 20 caracteres, matriz
de 5×7): driver no bloqueante y un motor de pantallas con alineación,
marquesinas, efectos de tipear y borrar, caracteres custom animados y
secuencias de pantallas.

Es C portable: corre en cualquier placa Arduino, en un 8051 (nació para la
BondiMachine, con un SAB 80C537 y 256 bytes de RAM interna) y en la PC, donde
se prueba. Incluye un ejemplo para ESP32 que convierte el display en un
cartel controlado desde el navegador.

## Uso

Todo es no bloqueante: las funciones encolan o avanzan un paso y vuelven
enseguida, así que el loop principal puede hacer otras cosas.

```c
#include <BondiVFD.h>

void setup() {
  vfd_begin();
  screen_begin();
  screen_play(&demo_show);      // la demo: un título y el intermedio de Pac-Man
}

void loop() {
  vfd_poll();                   // manda al display el próximo byte pendiente
  screen_poll();                // avanza animaciones y pantallas
  // otras tareas, mientras no bloqueen
}
```

### Líneas

Cada línea (`screen_line_t`) tiene:

- **Texto** en UTF-8. `{0}`..`{9}` son caracteres custom y `{{` una llave.
  Los acentos, la ñ y símbolos como ° ± ¢ ● → van al juego europeo del
  display, que no es Latin-1: la tabla está en `charset.py`, sacada de la
  figura 3 de la hoja de datos. En este display `\` se ve como ¥ y `|` como £.
- **Alineación:** izquierda, centro o derecha.
- **Desplazamiento:** fija, marquesina que da vueltas (`SCREEN_SCROLL_LEFT`
  o `_RIGHT`) o una sola pasada en la que el texto entra por un costado y
  sale por el otro (`SCREEN_PASS_LEFT` o `_RIGHT`).
- **Efecto**, sólo en líneas fijas: tipear (aparece de a una letra), borrar
  (desaparece de izquierda a derecha) o los dos en loop. El cursor puede ser
  un bloque que titila, ninguno o un carácter custom animado: así Pac-Man se
  come el texto o un fantasma lo va escribiendo.

```c
static const screen_line_t hola = { "Hola {0}", SCREEN_CENTER, SCREEN_STATIC, 0,
                                    SCREEN_FX_TYPE, 120, 0 };   // se tipea con {0} de cursor
screen_set_line(0, &hola);
```

Los textos y los frames no se copian: tienen que seguir existiendo mientras
se muestran (por ejemplo, `const`). Así el motor entra en la RAM de un 8051.

### Caracteres custom

Hasta 10, `{0}`..`{9}`, cada uno con varios frames que se alternan solos
(`screen_set_glyph()`, o la lista de un show). Cada frame distinto ocupa una
de las 10 ubicaciones de caracteres custom del display. Los frames iguales,
aunque sean de distintos caracteres, comparten ubicación: la demo usa 12
frames pero sólo 9 ubicaciones.

### Shows: secuencias de pantallas

Un show (`screen_show_t`) es la lista de caracteres custom más una secuencia
de páginas que se repite. Cada página tiene sus dos líneas y una duración en
ms; con 0 dura hasta que terminan sus pasadas y efectos de una sola vez. Una
línea con texto `NULL` queda como estaba en la página anterior. La librería
trae dos shows escritos así, sin código:

- **`demo_show`** (`src/demo.c`): un título que se tipea y el intermedio de
  Pac-Man, con los fantasmas persiguiéndolo y después al revés.
- **`doom_show`** (`src/doom.c`): la barra de estado del Doom, con munición,
  salud, armadura, llaves y la cara del Doomguy (2×2 caracteres) mirando a
  los costados mientras dispara, recibe golpes y levanta armadura.

Los frames de varios caracteres que cambian a la vez (como las dos mitades
de la cara) cambian en la misma vuelta, así el dibujo nunca queda partido.

### Doom en vivo

`doom_live.c` muestra la barra del Doom con el estado que manda el juego:
munición, salud, armadura, llaves y la cara que elige el propio Doom
(`st_faceindex`: nivel de dolor, mirando a los costados, ouch, sonrisa
malvada, furia, modo dios y muerto; los dibujos están en `doom_faces.py`).
Sólo reescribe lo que cambia y recarga únicamente los caracteres de la cara
que cambiaron.

```c
doom_status_t s = { 47, 61, 100, DOOM_KEY_BLUE | DOOM_KEY_RED, 12 };
doom_live_begin();                                  // una vez
doom_live_update(&s);                               // cada vez que cambia algo
doom_live_parse("ammo=46&face=13", &s);             // o desde texto
```

`tools/chocolate-doom` tiene un parche para Chocolate Doom que manda ese
estado por UDP a BondiWeb mientras se juega.

### Driver

`vfd_write()`, `vfd_puts_at()`, `vfd_goto()` y `vfd_clear()` encolan bytes
(hasta `VFD_QUEUE_SIZE`, 40 por defecto) y devuelven 0 si no hay lugar.
`vfd_idle()` indica que terminó la inicialización y no queda nada pendiente.

## Estructura

| Ruta | Qué es |
|---|---|
| `src/vfd.c`, `vfd.h` | Driver del display: reset, comandos, cola de bytes |
| `src/vfd_char.c` | Carga de caracteres custom |
| `src/screen.c`, `screen.h` | Líneas, desplazamientos, efectos, caracteres custom animados y shows |
| `src/demo.c`, `demo.h` | La demo de Pac-Man, como show (sólo configuración) |
| `src/doom.c`, `doom.h` | La barra de estado del Doom, como show |
| `src/doom_live.c`, `doom_live.h` | La barra del Doom en vivo, con el estado del juego |
| `src/doom_faces.h`, `doom_faces.py` | Las caras del Doomguy para el modo en vivo |
| `tools/chocolate-doom/` | Parche para Chocolate Doom que manda la barra al display |
| `src/vfd_hal.h` | Lo que cada plataforma tiene que implementar (6 funciones) |
| `src/hal_arduino.cpp`, `BondiVFD.h` | HAL y header para Arduino |
| `src/glyphs.h`, `glyphs.py` | Dibujos de Pac-Man y mapeo de puntos de la hoja de datos |
| `src/charset.h`, `charset.py` | Fuente 5×7 del display y tabla Unicode → código |
| `examples/Demo/` | La demo en cualquier Arduino |
| `examples/BondiWeb/` | Cartel controlado desde el navegador (ESP32) |
| `examples/PinTest/` | Diagnóstico del cableado |
| `ports/host/` | HAL para PC y pruebas |
| `tools/vfd_model.py` | Modelo del display para las pruebas |
| `embed_html.py` | Genera `examples/BondiWeb/index_html.h` a partir de `index.html` |

Esta carpeta es una librería Arduino (`library.properties`).

## Arduino

Copiar o enlazar esta carpeta en `~/Documents/Arduino/libraries/BondiVFD` y
abrir el ejemplo **BondiVFD → Demo**. Con `arduino-cli`:

```bash
arduino-cli compile -b arduino:avr:uno --library . examples/Demo
```

Conexión por defecto (se cambia con `vfd_set_pins()`; el ejemplo asigna
cada pin por nombre):

| VFD (J1) | Arduino | ESP32 DevKit (30 pines, WROOM-32) |
|---|---|---|
| D0..D7 (J1-10..J1-3) | 2..9 | D15, D2, D4, RX2, TX2, D5, D18, D19 |
| WRITE STROBE (J1-2) | 10 | D21 |
| RESET (J1-14) | 11 | D22 |
| BUSY (J1-1) | 12 | D23 |
| +5 V (J1-11), GND (J1-12) | fuente de 5 V (~370 mA), GND | ídem |

El display es de 5 V. Con placas de 3,3 V (ESP32, RP2040, SAMD...) hace falta
un conversor de nivel, como mínimo en BUSY, que sale con hasta 5 V.

Después de cada byte nunca se espera más de 750 µs, el tiempo máximo de
ejecución de la hoja de datos, aunque BUSY no baje. BUSY y RESET se pueden
dejar sin conectar con `VFD_NO_PIN`: sin BUSY se espera siempre ese máximo
(cada línea tarda unos 17 ms en escribirse en lugar de 5). Sin RESET alcanza
con el reset por software que manda `vfd_begin()`.

### ESP32

- **Pines:** la columna derecha del DevKit de 30 pines, de D15 hacia
  arriba, salteando RX0/TX0, así que `Serial` queda libre. RX2/TX2 (GPIO
  16/17) son de la PSRAM en los módulos WROVER: este pinout es para WROOM-32.
  Para S2, S3 o C3 elegir pines con `vfd_set_pins()`.
- **D2 (GPIO2) y la carga del programa:** para entrar en modo de carga, GPIO2
  tiene que estar en bajo o al aire, y el display le pone un pull-up a 5 V.
  Si al subir el programa falla con "Failed to connect", desenchufar la
  alimentación del display mientras se carga.
- **Alimentación:** si el display toma los 5 V del pin VIN del DevKit, el
  ESP32 puede reiniciarse cuando el display consume más. Mejor una fuente de
  5 V aparte, con GND común.
- **Diagnóstico:** `examples/PinTest` verifica la continuidad de cada
  línea, busca cortos, mide BUSY y escribe letras que prueban cada bit.

## BondiWeb (ESP32)

`examples/BondiWeb` convierte el ESP32 en un cartel controlado desde el
navegador: una secuencia de pantallas, cada una con sus dos líneas
(alineación, marquesina o pasada en cualquier dirección, efectos de tipear y
borrar) y su duración, más un editor de caracteres custom con varios frames
y una galería de animaciones. La primera vez arranca con la demo; desde la
página se pueden cargar la demo de Pac-Man o la barra del Doom. Todo se
guarda en la flash.

- **WiFi:** la primera vez, o si no logra conectarse, el ESP32 crea la red
  `BondiMachine` (clave `bondimachine`). Entrando a <http://192.168.4.1/> se
  carga la red de la casa. Al arrancar, el display muestra la IP durante 5 s;
  también responde en <http://bondi.local/>.
- **Red por defecto:** copiar `wifi_config.h.example` como `wifi_config.h`
  (no se sube a git) con la red de la casa.
- **Página en el ESP32 o local:** el ESP32 sirve la página en `/`. La API
  tiene CORS, así que el mismo `index.html` se puede abrir desde la compu
  escribiendo la IP del ESP32 arriba.
- **Editar la página:** después de cambiar `examples/BondiWeb/index.html`,
  correr `make web`. La vista previa dibuja con la fuente real del display y
  reproduce la secuencia con los mismos tiempos que `screen.c`.

API (POST con `application/x-www-form-urlencoded`, respuestas JSON):

| Ruta | Parámetros |
|---|---|
| `GET /api/state` | Devuelve las pantallas, los caracteres custom, la pantalla actual, ubicaciones en uso y WiFi |
| `POST /api/lines` | Cambia una pantalla: `page` (0.., o la cantidad para agregar una), `ms` (duración, 0 = hasta que terminen sus animaciones) y por línea `text1`, `align1` (`left`, `center`, `right`), `scroll1` (`static`, `left`, `right`, `pass-left`, `pass-right`), `speed1` (ms por paso), `effect1` (`none`, `type`, `erase`, `cycle`), `fxms1` (ms por letra), `cursor1` (`block`, `none` o `0`-`9`), `keep1` (`1` = igual que la pantalla anterior), y/o lo mismo con `2`. Sólo cambia lo que se manda |
| `POST /api/pages` | Reemplaza toda la secuencia: `n` y, por cada pantalla `i`, los mismos campos con el prefijo `p<i>_` (`p0_ms`, `p0_text1`...) |
| `POST /api/glyph` | `id` (0-9), `frames` (separados por coma, cada uno 7 filas de 2 dígitos hex; vacío = borrar), `ms` (cada cuánto cambia de frame) |
| `POST /api/demo` | Carga la demo de Pac-Man |
| `POST /api/doom` | Carga la barra de estado del Doom |
| `POST /api/doom/live` | Barra del Doom en vivo: `ammo` (`-1` = sin munición), `health`, `armor`, `keys` (letras `b`, `y`, `r`), `face` (`st_faceindex` de Doom, 0-41). No se guarda en la flash; a los 10 s sin datos vuelve al show. El mismo texto (`ammo=47&health=61...`) se puede mandar por UDP al puerto 5050 |
| `POST /api/wifi` | `ssid`, `pass`: guarda y reinicia |

Por ejemplo:

```bash
curl -d "text1=Hola {0}&align1=center&scroll1=static" http://bondi.local/api/lines
```
```bash
curl -d "text2=Coloque monedas&scroll2=right&speed2=150" http://bondi.local/api/lines
```
```bash
curl -d "id=0&ms=250&frames=0E1E1C181C1E0E,0E1F1F1F1F1F0E" http://bondi.local/api/glyph
```
```bash
curl -d "ammo=47&health=61&armor=100&keys=br&face=13" http://bondi.local/api/doom/live
```

## Otra plataforma

Implementar las funciones de `src/vfd_hal.h` (ver `ports/host/hal_host.c`
como ejemplo mínimo) y compilar los `.c` de `src/` junto con el HAL. La
única espera activa es `vfd_hal_wait_20us()`; el tiempo se mide con
`vfd_hal_millis()`, de 16 bits (puede dar la vuelta).

Con poca RAM se pueden achicar `SCREEN_GLYPHS` (caracteres custom) y
`VFD_QUEUE_SIZE` (mínimo 22, una línea entera). En un 8051 con SDCC conviene
compilar todo con `--stack-auto`, para que las variables locales vayan a la
pila en vez de ocupar RAM fija.

## Pruebas

```bash
make check
```

- **Demo:** la compila para la PC (`-Wall -Wextra -Wconversion -Werror`),
  reproduce lo que manda al display en el modelo y verifica la historia:
  Pac-Man perseguido hacia la izquierda, los fantasmas asustados hacia la
  derecha y la pantalla final, reconociendo a los personajes por el dibujo
  cargado en el display.
- **Doom:** verifica que la línea de abajo quede fija, que los números
  pasen en orden, que la cara mire a los tres lados sin quedar partida y
  que use 9 ubicaciones.
- **Doom en vivo:** pasa una serie de estados del juego, en el mismo texto
  que manda el parche, y verifica los números, las llaves y que cada cara
  cargada en el display sea la que corresponde.
- **Pantallas:** corre `screen.c` con un reloj simulado y verifica
  alineación, acentos, marquesinas en los dos sentidos, pasadas, efectos,
  shows con varias páginas y que los frames iguales compartan ubicación.
