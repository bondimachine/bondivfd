/*
 * PinTest.ino - Diagnostico del cableado del VFD IEE 03601-95B-40.
 * Usa los pines por defecto de BondiVFD.h. Resultados por Serial a 115200.
 *
 * 1. Continuidad: cada entrada del display (D0-D7, WR, RESET) tiene un
 *    pull-up de 10 kOhm a 5 V. Con el pull-down interno del pin, una linea
 *    conectada se lee en alto y una suelta en bajo. (Solo en placas con
 *    INPUT_PULLDOWN, como el ESP32.)
 * 2. Cortos: se pone cada linea en bajo y se mira que las demas sigan altas.
 * 3. BUSY: nivel durante y despues del reset por hardware.
 * 4. Escritura: manda "@ABDHP`" (0x40 y un bit mas cada letra) y mide si
 *    BUSY sube despues de cada strobe. Mirar que se ve en el display.
 */
#include <BondiVFD.h>
#include <vfd_hal.h>

static const vfd_pins_t P = VFD_DEFAULT_PINS;

struct Line { const char *name; uint8_t pin; };
static const Line lines[] = {
  { "D0", P.d0 }, { "D1", P.d1 }, { "D2", P.d2 }, { "D3", P.d3 },
  { "D4", P.d4 }, { "D5", P.d5 }, { "D6", P.d6 }, { "D7", P.d7 },
  { "WR", P.wr }, { "RESET", P.reset },
};
static const uint8_t N = sizeof lines / sizeof lines[0];

static void release_all() {
  for (uint8_t i = 0; i < N; i++)
    pinMode(lines[i].pin, INPUT_PULLDOWN);
  delay(5);
}

static void continuity() {
  Serial.println("\n== 1. Continuidad (alto = llega al display) ==");
  release_all();
  for (uint8_t i = 0; i < N; i++) {
    int v = digitalRead(lines[i].pin);
    Serial.printf("  %-5s GPIO%-2u : %s\n", lines[i].name, lines[i].pin,
                  v ? "ALTO  ok" : "BAJO  <-- suelto o display sin 5 V");
  }
  if (P.busy != VFD_NO_PIN) {
    pinMode(P.busy, INPUT_PULLDOWN);
    Serial.printf("  BUSY  GPIO%-2u : %s\n", P.busy, digitalRead(P.busy) ? "ALTO (ocupado)" : "BAJO (libre)");
  }
}

/* Solo entre lineas que dieron ALTO en la prueba de continuidad: si una ya
 * esta en bajo no se puede saber si otra la arrastra. */
static void shorts() {
  Serial.println("\n== 2. Cortos entre lineas ==");
  bool high[N];
  bool any = false;
  release_all();
  for (uint8_t i = 0; i < N; i++)
    high[i] = digitalRead(lines[i].pin);
  for (uint8_t i = 0; i < N; i++) {
    if (!high[i])
      continue;
    release_all();
    pinMode(lines[i].pin, OUTPUT);
    digitalWrite(lines[i].pin, LOW);
    delay(2);
    for (uint8_t j = 0; j < N; j++) {
      if (j != i && high[j] && !digitalRead(lines[j].pin)) {
        Serial.printf("  %s (GPIO%u) en bajo arrastra a %s (GPIO%u)\n",
                      lines[i].name, lines[i].pin, lines[j].name, lines[j].pin);
        any = true;
      }
    }
  }
  if (!any)
    Serial.println("  ninguno");
  release_all();
}

static void busy_trace(const char *what, uint16_t ms) {
  Serial.printf("  %s: ", what);
  int last = -1;
  uint32_t t0 = millis();
  while (millis() - t0 < ms) {
    int v = digitalRead(P.busy);
    if (v != last) {
      Serial.printf("[%lu ms %s] ", (unsigned long)(millis() - t0), v ? "ALTO" : "BAJO");
      last = v;
    }
  }
  Serial.println();
}

static void reset_and_write() {
  vfd_set_pins(&P);
  vfd_hal_init();

  Serial.println("\n== 3. BUSY con reset por hardware ==");
  if (P.busy == VFD_NO_PIN) {
    Serial.println("  BUSY sin conectar, se saltea");
  } else {
    vfd_hal_reset(1);
    busy_trace("RESET bajo", 20);
    vfd_hal_reset(0);
    busy_trace("RESET liberado, 1 s", 1000);
  }
  if (P.busy == VFD_NO_PIN || P.reset == VFD_NO_PIN)
    delay(600);

  Serial.println("\n== 4. Escritura de \"@ABDHP`\" (deberia verse arriba a la izquierda) ==");
  const char *s = "@ABDHP`";
  for (const char *c = s; *c; c++) {
    uint32_t t0 = micros();
    while (vfd_hal_busy() && micros() - t0 < 5000) {}
    bool was_busy = vfd_hal_busy();
    delayMicroseconds(25);
    vfd_hal_strobe((uint8_t)*c);
    uint32_t ts = micros();
    bool rose = false;
    uint32_t busy_us = 0;
    if (P.busy != VFD_NO_PIN) {
      while (micros() - ts < 5000) {
        if (digitalRead(P.busy)) { rose = true; }
        else if (rose) { busy_us = micros() - ts; break; }
      }
    }
    Serial.printf("  '%c' 0x%02X: BUSY antes %s, despues %s",
                  *c, *c, was_busy ? "ALTO (timeout)" : "bajo",
                  P.busy == VFD_NO_PIN ? "-" : rose ? "subio" : "NO SUBIO");
    if (busy_us)
      Serial.printf(", bajo a los %lu us", (unsigned long)busy_us);
    Serial.println();
  }
}

void setup() {
  Serial.begin(115200);
  delay(500);
}

void loop() {
  Serial.println("\n######## PinTest BondiVFD ########");
  continuity();
  shorts();
  reset_and_write();
  Serial.println("\n(se repite en 15 s)");
  delay(15000);
}
