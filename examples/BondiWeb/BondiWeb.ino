/*
 * BondiWeb.ino - Mensajes en el VFD IEE 03601-95B-40 desde el navegador
 * (ESP32).
 *
 * El ESP32 sirve la pagina en http://<ip>/ (o http://bondi.local/) y una API
 * con CORS, asi que la misma pagina (index.html) tambien se puede abrir desde
 * la compu y apuntar al ESP32.
 *
 * Lo que se muestra es una secuencia de pantallas (un show de screen.h) que
 * se repite: cada pantalla tiene sus dos lineas y cuanto dura. La primera vez
 * arranca con la demo de la libreria (src/demo.c).
 *
 * WiFi: usa la red de wifi_config.h (ver wifi_config.h.example) o la cargada
 * desde la pagina. Si no hay ninguna o no logra conectarse, crea la red
 * "BondiMachine" (clave "bondimachine"); la pagina en http://192.168.4.1/
 * permite cargar la red de la casa. Al arrancar, el display muestra la IP
 * durante 5 segundos.
 *
 * API (POST con application/x-www-form-urlencoded, respuestas JSON):
 *   GET  /api/state   pantallas, caracteres custom y WiFi
 *   POST /api/lines   cambia lineas de una pantalla: page (0..), ms, y por
 *                     linea text1, align1 (left|center|right), scroll1
 *                     (static|left|right|pass-left|pass-right), speed1 (ms por
 *                     paso), effect1 (none|type|erase|cycle), fxms1 (ms por
 *                     letra), cursor1 (block|none|0-9), keep1 (1 = igual que
 *                     la pantalla anterior), y/o lo mismo con 2. Solo cambia
 *                     lo que se manda; page = cantidad de pantallas agrega una.
 *   POST /api/pages   reemplaza toda la secuencia: n, y para cada pantalla i
 *                     p<i>_ms, p<i>_text1, p<i>_align1... (mismos campos)
 *   POST /api/glyph   id (0-9), frames (frames separados por coma, cada uno 7
 *                     filas de 2 digitos hex, bits 4..0 = columnas de izq. a
 *                     der.; vacio = borrar), ms (cada cuanto cambia de frame)
 *   POST /api/demo    carga la demo de la libreria
 *   POST /api/wifi    ssid, pass (guarda y reinicia)
 * En el texto, {0}..{9} son los caracteres custom y {{ es una llave.
 *
 * Usa los pines por defecto de BondiVFD.h (o vfd_set_pins() en setup()).
 */
#include <WiFi.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <Preferences.h>
#include <BondiVFD.h>
#include "index_html.h"

/* Red por defecto: copiar wifi_config.h.example como wifi_config.h. La red
 * cargada desde la pagina tiene prioridad. */
#if __has_include("wifi_config.h")
#include "wifi_config.h"
#endif
#ifndef WIFI_SSID
#define WIFI_SSID ""
#define WIFI_PASS ""
#endif

#define AP_SSID      "BondiMachine"
#define AP_PASS      "bondimachine"
#define HOSTNAME     "bondi"
#define CONNECT_MS   15000
#define SHOW_IP_MS   5000
#define MAX_PAGES    12
#define MAX_FRAMES   10         /* por caracter custom */

struct LineCfg {
  bool keep;                    /* igual que en la pantalla anterior */
  String text;
  uint8_t align;
  uint8_t scroll;
  uint16_t speed;
  uint8_t effect;
  uint16_t fxms;
  uint8_t cursor;               /* 0-9, SCREEN_CURSOR_BLOCK o SCREEN_CURSOR_NONE */
};

struct PageCfg {
  LineCfg line[SCREEN_LINES];
  uint16_t ms;
};

struct GlyphCfg {
  uint8_t n;
  uint16_t ms;
  uint8_t rows[MAX_FRAMES][7];
};

static WebServer server(80);
static Preferences prefs;
static PageCfg pages[MAX_PAGES];
static uint8_t n_pages;
static GlyphCfg glyphs[SCREEN_GLYPHS];

/* El show que reproduce screen.c apunta a los textos y frames de arriba */
static screen_page_t show_pages[MAX_PAGES];
static screen_glyph_t show_glyphs[SCREEN_GLYPHS];
static screen_show_t show;

static uint32_t show_ip_until;
static uint32_t restart_at;

static const char *ALIGN_NAMES[] = { "left", "center", "right" };
static const char *SCROLL_NAMES[] = { "static", "left", "right", "pass-left", "pass-right" };
static const char *EFFECT_NAMES[] = { "none", "type", "erase", "cycle" };

/* ---- Show --------------------------------------------------------------- */

/* Arma el show con la configuracion y lo reproduce desde la primera
 * pantalla. Devuelve false si los frames no entran en el display. */
static bool play() {
  uint8_t n_g = 0;
  for (uint8_t g = 0; g < SCREEN_GLYPHS; g++)
    if (glyphs[g].n)
      show_glyphs[n_g++] = { g, glyphs[g].n, glyphs[g].ms, &glyphs[g].rows[0][0] };
  for (uint8_t p = 0; p < n_pages; p++) {
    for (uint8_t i = 0; i < SCREEN_LINES; i++) {
      const LineCfg &c = pages[p].line[i];
      show_pages[p].line[i] = { c.keep ? nullptr : c.text.c_str(), c.align, c.scroll, c.speed,
                                c.effect, c.fxms, c.cursor };
    }
    show_pages[p].ms = pages[p].ms;
  }
  show = { show_glyphs, n_g, show_pages, n_pages };
  if (show_ip_until)
    return true;                /* se reproduce cuando termina de mostrar la IP */
  return screen_play(&show);
}

/* Copia un show de C (como demo_show) a la configuracion editable */
static void load_show(const screen_show_t *s) {
  memset(glyphs, 0, sizeof glyphs);
  for (uint8_t i = 0; i < s->n_glyphs; i++) {
    const screen_glyph_t &d = s->glyphs[i];
    if (d.id >= SCREEN_GLYPHS) continue;
    GlyphCfg &g = glyphs[d.id];
    g.n = min<uint8_t>(d.n_frames, MAX_FRAMES);
    g.ms = d.frame_ms;
    memcpy(g.rows, d.frames, g.n * 7);
  }
  n_pages = min<uint8_t>(s->n_pages, MAX_PAGES);
  for (uint8_t p = 0; p < n_pages; p++) {
    for (uint8_t i = 0; i < SCREEN_LINES; i++) {
      const screen_line_t &l = s->pages[p].line[i];
      pages[p].line[i] = { l.text == nullptr, l.text ? l.text : "", l.align, l.scroll,
                           l.scroll_ms ? l.scroll_ms : (uint16_t)200, l.effect,
                           l.effect_ms ? l.effect_ms : (uint16_t)120,
                           l.effect ? l.cursor : (uint8_t)SCREEN_CURSOR_BLOCK };
    }
    pages[p].ms = s->pages[p].ms;
  }
}

/* ---- Persistencia ------------------------------------------------------- */

struct LineBin {                /* campos fijos de una linea, para la flash */
  uint8_t keep, align, scroll, effect, cursor;
  uint16_t speed, fxms;
};

static void save_pages() {
  prefs.putUChar("np", n_pages);
  for (uint8_t p = 0; p < n_pages; p++) {
    LineBin bin[SCREEN_LINES];
    for (uint8_t i = 0; i < SCREEN_LINES; i++) {
      const LineCfg &c = pages[p].line[i];
      bin[i] = { c.keep, c.align, c.scroll, c.effect, c.cursor, c.speed, c.fxms };
      prefs.putString(("t" + String(p) + "_" + i).c_str(), c.text);
    }
    prefs.putBytes(("b" + String(p)).c_str(), bin, sizeof bin);
    prefs.putUShort(("m" + String(p)).c_str(), pages[p].ms);
  }
}

static void save_glyphs() {
  prefs.putBytes("g2", glyphs, sizeof glyphs);
}

static void load_state() {
  if (!prefs.isKey("np")) {
    load_show(&demo_show);
    return;
  }
  n_pages = min<uint8_t>(prefs.getUChar("np", 0), MAX_PAGES);
  for (uint8_t p = 0; p < n_pages; p++) {
    LineBin bin[SCREEN_LINES] = {};
    prefs.getBytes(("b" + String(p)).c_str(), bin, sizeof bin);
    for (uint8_t i = 0; i < SCREEN_LINES; i++)
      pages[p].line[i] = { (bool)bin[i].keep, prefs.getString(("t" + String(p) + "_" + i).c_str(), ""),
                           bin[i].align, bin[i].scroll, bin[i].speed, bin[i].effect, bin[i].fxms,
                           bin[i].cursor };
    pages[p].ms = prefs.getUShort(("m" + String(p)).c_str(), 0);
  }
  memset(glyphs, 0, sizeof glyphs);
  if (prefs.getBytesLength("g2") == sizeof glyphs)
    prefs.getBytes("g2", glyphs, sizeof glyphs);
}

/* ---- HTTP --------------------------------------------------------------- */

static void cors() {
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.sendHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
  server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
  // Chrome pide esto para llamar a una IP privada desde otra pagina
  server.sendHeader("Access-Control-Allow-Private-Network", "true");
}

static String json_str(const String &s) {
  String out = "\"";
  for (size_t i = 0; i < s.length(); i++) {
    char c = s[i];
    if (c == '"' || c == '\\') {
      out += '\\';
      out += c;
    } else if ((uint8_t)c < 0x20) {
      char buf[8];
      snprintf(buf, sizeof buf, "\\u%04x", c);
      out += buf;
    } else {
      out += c;
    }
  }
  return out + "\"";
}

static void send_json(int code, const String &body) {
  cors();
  server.send(code, "application/json", body);
}

static void send_error(int code, const String &msg) {
  send_json(code, "{\"error\":" + json_str(msg) + "}");
}

static String cursor_name(uint8_t c) {
  if (c == SCREEN_CURSOR_BLOCK) return "block";
  if (c == SCREEN_CURSOR_NONE) return "none";
  return String(c);
}

static void handle_state() {
  String j = "{\"pages\":[";
  for (uint8_t p = 0; p < n_pages; p++) {
    j += String(p ? "," : "") + "{\"ms\":" + pages[p].ms + ",\"lines\":[";
    for (uint8_t i = 0; i < SCREEN_LINES; i++) {
      const LineCfg &c = pages[p].line[i];
      j += String(i ? "," : "") + "{\"keep\":" + (c.keep ? "true" : "false") +
           ",\"text\":" + json_str(c.text) + ",\"align\":\"" + ALIGN_NAMES[c.align] +
           "\",\"scroll\":\"" + SCROLL_NAMES[c.scroll] + "\",\"speed\":" + c.speed +
           ",\"effect\":\"" + EFFECT_NAMES[c.effect] + "\",\"fxms\":" + c.fxms +
           ",\"cursor\":\"" + cursor_name(c.cursor) + "\"}";
    }
    j += "]}";
  }
  j += "],\"glyphs\":[";
  bool first = true;
  for (uint8_t g = 0; g < SCREEN_GLYPHS; g++) {
    if (!glyphs[g].n) continue;
    j += String(first ? "" : ",") + "{\"id\":" + g + ",\"ms\":" + glyphs[g].ms + ",\"frames\":[";
    first = false;
    for (uint8_t f = 0; f < glyphs[g].n; f++) {
      char hex[15];
      for (uint8_t r = 0; r < 7; r++)
        snprintf(hex + r * 2, 3, "%02X", glyphs[g].rows[f][r]);
      j += String(f ? "," : "") + "\"" + hex + "\"";
    }
    j += "]}";
  }
  bool ap = WiFi.getMode() & WIFI_AP;
  j += "],\"page\":" + String(screen_page());
  j += ",\"slots\":{\"used\":" + String(screen_slots_used()) + ",\"total\":" + VFD_UDC_SLOTS + "}";
  j += ",\"wifi\":{\"mode\":\"" + String(ap ? "ap" : "sta") + "\",\"ssid\":" +
       json_str(ap ? String(AP_SSID) : WiFi.SSID()) + ",\"ip\":\"" +
       (ap ? WiFi.softAPIP() : WiFi.localIP()).toString() + "\"}}";
  send_json(200, j);
}

static bool parse_name(const String &s, const char *const *names, uint8_t n, uint8_t *out) {
  for (uint8_t i = 0; i < n; i++)
    if (s == names[i]) { *out = i; return true; }
  return false;
}

static bool parse_cursor(const String &s, uint8_t *out) {
  if (s == "block") { *out = SCREEN_CURSOR_BLOCK; return true; }
  if (s == "none") { *out = SCREEN_CURSOR_NONE; return true; }
  if (s.length() == 1 && isDigit(s[0])) { *out = s[0] - '0'; return true; }
  return false;
}

/* Lee los campos de una linea con el prefijo dado (por ejemplo "p0_" y "1"
 * para p0_text1, p0_align1...). Devuelve "" o el error. */
static String parse_line(const String &pre, const String &n, LineCfg *c, bool *any) {
  auto has = [&](const char *k) { return server.hasArg(pre + k + n); };
  auto arg = [&](const char *k) { return server.arg(pre + k + n); };
  if (has("keep")) { c->keep = arg("keep") == "1" || arg("keep") == "true"; *any = true; }
  if (has("text")) { c->text = arg("text"); *any = true; }
  if (has("align")) {
    if (!parse_name(arg("align"), ALIGN_NAMES, 3, &c->align))
      return pre + "align" + n + " tiene que ser left, center o right";
    *any = true;
  }
  if (has("scroll")) {
    if (!parse_name(arg("scroll"), SCROLL_NAMES, 5, &c->scroll))
      return pre + "scroll" + n + " tiene que ser static, left, right, pass-left o pass-right";
    *any = true;
  }
  if (has("speed")) {
    long ms = arg("speed").toInt();
    if (ms < 20 || ms > 5000)
      return pre + "speed" + n + " tiene que estar entre 20 y 5000 ms";
    c->speed = (uint16_t)ms;
    *any = true;
  }
  if (has("effect")) {
    if (!parse_name(arg("effect"), EFFECT_NAMES, 4, &c->effect))
      return pre + "effect" + n + " tiene que ser none, type, erase o cycle";
    *any = true;
  }
  if (has("fxms")) {
    long ms = arg("fxms").toInt();
    if (ms < 20 || ms > 5000)
      return pre + "fxms" + n + " tiene que estar entre 20 y 5000 ms";
    c->fxms = (uint16_t)ms;
    *any = true;
  }
  if (has("cursor")) {
    if (!parse_cursor(arg("cursor"), &c->cursor))
      return pre + "cursor" + n + " tiene que ser block, none o 0-9";
    *any = true;
  }
  return "";
}

static String parse_page(const String &pre, PageCfg *p, bool *any) {
  if (server.hasArg(pre + "ms")) {
    long ms = server.arg(pre + "ms").toInt();
    if (ms < 0 || ms > 60000)
      return pre + "ms tiene que estar entre 0 y 60000";
    p->ms = (uint16_t)ms;
    *any = true;
  }
  for (uint8_t i = 0; i < SCREEN_LINES; i++) {
    String err = parse_line(pre, String(i + 1), &p->line[i], any);
    if (err.length()) return err;
  }
  return "";
}

static PageCfg default_page() {
  PageCfg p;
  for (uint8_t i = 0; i < SCREEN_LINES; i++)
    p.line[i] = { false, "", SCREEN_LEFT, SCREEN_STATIC, 200, SCREEN_FX_NONE, 120, SCREEN_CURSOR_BLOCK };
  p.ms = 5000;
  return p;
}

static void apply_and_reply() {
  if (!play()) {
    send_error(409, "los frames de los caracteres custom no entran en las 10 ubicaciones del display");
    return;
  }
  save_pages();
  save_glyphs();
  handle_state();
}

static void handle_lines() {
  long p = server.hasArg("page") ? server.arg("page").toInt() : 0;
  if (p < 0 || p > n_pages || p >= MAX_PAGES)
    return send_error(400, "page tiene que ser de 0 a " + String(min<int>(n_pages, MAX_PAGES - 1)));
  PageCfg next = p < n_pages ? pages[p] : default_page();
  bool any = false;
  String err = parse_page("", &next, &any);
  if (err.length()) return send_error(400, err);
  if (!any) return send_error(400, "no hay nada para cambiar (text1, align1, scroll1, ms...)");
  pages[p] = next;
  if (p == n_pages) n_pages++;
  show_ip_until = 0;
  apply_and_reply();
}

static void handle_pages() {
  long n = server.arg("n").toInt();
  if (n < 1 || n > MAX_PAGES)
    return send_error(400, "n tiene que ser de 1 a " + String(MAX_PAGES));
  static PageCfg next[MAX_PAGES];
  for (uint8_t p = 0; p < n; p++) {
    next[p] = default_page();
    bool any = false;
    String err = parse_page("p" + String(p) + "_", &next[p], &any);
    if (err.length()) return send_error(400, err);
  }
  for (uint8_t p = 0; p < n; p++)
    pages[p] = next[p];
  n_pages = (uint8_t)n;
  show_ip_until = 0;
  apply_and_reply();
}

static int hexval(char c) {
  if (c >= '0' && c <= '9') return c - '0';
  if (c >= 'a' && c <= 'f') return c - 'a' + 10;
  if (c >= 'A' && c <= 'F') return c - 'A' + 10;
  return -1;
}

static void handle_glyph() {
  String sid = server.arg("id");
  if (sid.length() != 1 || !isDigit(sid[0]))
    return send_error(400, "id tiene que ser de 0 a 9");
  uint8_t id = sid[0] - '0';
  String frames = server.arg("frames");
  frames.replace(" ", "");
  GlyphCfg g = {};
  g.ms = (uint16_t)server.arg("ms").toInt();
  int pos = 0;
  while (pos < (int)frames.length()) {
    int end = frames.indexOf(',', pos);
    if (end < 0) end = frames.length();
    String f = frames.substring(pos, end);
    if (g.n == MAX_FRAMES || f.length() != 14)
      return send_error(400, "cada frame son 7 filas de 2 digitos hex, hasta " + String(MAX_FRAMES) + " frames");
    for (uint8_t r = 0; r < 7; r++) {
      int hi = hexval(f[r * 2]), lo = hexval(f[r * 2 + 1]);
      if (hi < 0 || lo < 0)
        return send_error(400, "frame con caracteres que no son hex: " + f);
      g.rows[g.n][r] = (uint8_t)(hi << 4 | lo) & 0x1F;
    }
    g.n++;
    pos = end + 1;
  }
  if (g.n > 1 && (g.ms < 30 || g.ms > 10000))
    return send_error(400, "ms tiene que estar entre 30 y 10000 con mas de un frame");
  GlyphCfg old = glyphs[id];
  glyphs[id] = g;
  if (!play()) {
    glyphs[id] = old;
    play();
    return send_error(409, "los frames distintos no entran en las 10 ubicaciones del display");
  }
  save_glyphs();
  handle_state();
}

static void handle_demo() {
  load_show(&demo_show);
  show_ip_until = 0;
  apply_and_reply();
}

static void handle_wifi() {
  if (!server.hasArg("ssid") || server.arg("ssid").isEmpty())
    return send_error(400, "falta ssid");
  prefs.putString("ssid", server.arg("ssid"));
  prefs.putString("pass", server.arg("pass"));
  send_json(200, "{\"ok\":true,\"restarting\":true}");
  restart_at = millis() + 1000;
}

static void handle_not_found() {
  if (server.method() == HTTP_OPTIONS) {   // preflight de CORS
    cors();
    server.send(204);
    return;
  }
  send_error(404, "no existe " + server.uri());
}

/* ---- WiFi --------------------------------------------------------------- */

static void poll_display() {
  vfd_poll();
  screen_poll();
}

static void show_text(const char *top, const char *bottom) {
  static String t, b;                     /* screen.c guarda los punteros */
  t = top;
  b = bottom;
  screen_line_t l0 = { t.c_str(), SCREEN_CENTER, SCREEN_STATIC, 0, SCREEN_FX_NONE, 0, 0 };
  screen_line_t l1 = { b.c_str(), SCREEN_CENTER, SCREEN_STATIC, 0, SCREEN_FX_NONE, 0, 0 };
  screen_stop();
  screen_set_line(0, &l0);
  screen_set_line(1, &l1);
}

static void show_ip(const String &title, IPAddress ip) {
  show_text(title.c_str(), ip.toString().c_str());
  show_ip_until = millis() + SHOW_IP_MS;
}

static void start_wifi() {
  String ssid = prefs.getString("ssid", WIFI_SSID);
  String pass = prefs.isKey("ssid") ? prefs.getString("pass", "") : String(WIFI_PASS);
  if (ssid.length()) {
    WiFi.mode(WIFI_STA);
    WiFi.setHostname(HOSTNAME);
    WiFi.begin(ssid.c_str(), pass.c_str());
    show_text("Conectando a", ssid.c_str());
    uint32_t t0 = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - t0 < CONNECT_MS)
      poll_display();
    if (WiFi.status() == WL_CONNECTED) {
      Serial.printf("WiFi %s, IP %s\n", ssid.c_str(), WiFi.localIP().toString().c_str());
      show_ip(ssid, WiFi.localIP());
      return;
    }
    Serial.printf("No se pudo conectar a %s\n", ssid.c_str());
  }
  WiFi.mode(WIFI_AP);
  WiFi.softAP(AP_SSID, AP_PASS);
  Serial.printf("Red %s (clave %s), IP %s\n", AP_SSID, AP_PASS, WiFi.softAPIP().toString().c_str());
  show_ip("WiFi " AP_SSID, WiFi.softAPIP());
}

/* ---- Arduino ------------------------------------------------------------ */

void setup() {
  Serial.begin(115200);
  Serial.println("BondiWeb arrancando");

  prefs.begin("bondiweb", false);
  load_state();

  vfd_begin();
  screen_begin();
  start_wifi();
  play();                       /* arma el show; arranca al terminar de mostrar la IP */
  if (MDNS.begin(HOSTNAME))
    MDNS.addService("http", "tcp", 80);

  server.on("/", HTTP_GET, [] { server.send(200, "text/html", INDEX_HTML); });
  server.on("/api/state", HTTP_GET, handle_state);
  server.on("/api/lines", HTTP_POST, handle_lines);
  server.on("/api/pages", HTTP_POST, handle_pages);
  server.on("/api/glyph", HTTP_POST, handle_glyph);
  server.on("/api/demo", HTTP_POST, handle_demo);
  server.on("/api/wifi", HTTP_POST, handle_wifi);
  server.onNotFound(handle_not_found);
  server.begin();
}

void loop() {
  server.handleClient();
  poll_display();
  if (show_ip_until && (int32_t)(millis() - show_ip_until) >= 0) {
    show_ip_until = 0;
    if (!screen_play(&show))
      show_text("Caracteres custom", "no entran");
  }
  if (restart_at && (int32_t)(millis() - restart_at) >= 0)
    ESP.restart();
}
