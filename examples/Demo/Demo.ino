/*
 * Demo.ino - Demostracion de BondiVFD en un VFD IEE 03601-95B-40: un titulo
 * que se tipea y el intermedio de Pac-Man con caracteres custom.
 *
 * No bloquea: vfd_poll() y screen_poll() vuelven enseguida, asi que el
 * loop() puede hacer otras cosas (leer un monedero, botones, etc.).
 *
 * El display es de 5 V: con placas de 3,3 V usar un conversor de nivel,
 * sobre todo en BUSY. +5 V (J1-11) y GND (J1-12) a una fuente de 5 V de al
 * menos 500 mA, con GND comun con el Arduino.
 */
#include <BondiVFD.h>

void setup() {
  Serial.begin(115200);
  Serial.println("BondiVFD demo");

  vfd_pins_t pins;
#if defined(ARDUINO_ARCH_ESP32) && defined(CONFIG_IDF_TARGET_ESP32)
  // ESP32 DevKit de 30 pines (WROOM-32), columna derecha. RX0/TX0 libres.
  pins.busy  = 23;   // J1-1  D23
  pins.wr    = 21;   // J1-2  D21
  pins.d7    = 19;   // J1-3  D19
  pins.d6    = 18;   // J1-4  D18
  pins.d5    = 5;    // J1-5  D5
  pins.d4    = 17;   // J1-6  TX2 (GPIO17)
  pins.d3    = 16;   // J1-7  RX2 (GPIO16)
  pins.d2    = 4;    // J1-8  D4
  pins.d1    = 2;    // J1-9  D2
  pins.d0    = 15;   // J1-10 D15
  pins.reset = 22;   // J1-14 D22
#else
  pins.busy  = 12;   // J1-1  BUSY
  pins.wr    = 10;   // J1-2  WRITE STROBE
  pins.d7    = 9;    // J1-3
  pins.d6    = 8;    // J1-4
  pins.d5    = 7;    // J1-5
  pins.d4    = 6;    // J1-6
  pins.d3    = 5;    // J1-7
  pins.d2    = 4;    // J1-8
  pins.d1    = 3;    // J1-9
  pins.d0    = 2;    // J1-10
  pins.reset = 11;   // J1-14 RESET
#endif
  vfd_set_pins(&pins);

  vfd_begin();
  screen_begin();
  screen_play(&demo_show);      // un show de screen.h: solo configuracion (src/demo.c)
}

void loop() {
  vfd_poll();
  screen_poll();
  // aca puede ir cualquier otra tarea, mientras no bloquee
}
