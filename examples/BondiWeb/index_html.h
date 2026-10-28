/* Generado por embed_html.py a partir de index.html - no editar a mano */
#pragma once

static const char INDEX_HTML[] = R"html(<!doctype html>
<html lang="es">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>BondiMachine VFD</title>
<style>
:root {
--bg: #0f1211; --panel: #171c1a; --edge: #2a3330; --fg: #e4ece9; --muted: #8a9a94;
--vfd: #43f3c4; --vfd-off: #15241f; --accent: #43f3c4; --err: #ff7a6b; --ok: #43f3c4;
color-scheme: dark;
}
* { box-sizing: border-box; }
body { margin: 0; background: var(--bg); color: var(--fg); font: 15px/1.45 system-ui, sans-serif; }
main { max-width: 760px; margin: 0 auto; padding: 16px; display: grid; gap: 16px; }
h1 { font-size: 18px; margin: 0; letter-spacing: .02em; }
h2 { font-size: 15px; margin: 0 0 10px; color: var(--muted); font-weight: 600; text-transform: uppercase; letter-spacing: .06em; }
section { background: var(--panel); border: 1px solid var(--edge); border-radius: 10px; padding: 14px; }
.row { display: flex; flex-wrap: wrap; gap: 8px; align-items: center; }
.grow { flex: 1 1 200px; min-width: 0; }
input[type=text], input[type=number], input[type=password], select {
background: var(--bg); color: var(--fg); border: 1px solid var(--edge); border-radius: 6px;
padding: 8px 10px; font: inherit; min-width: 0;
}
input[type=text].mono { font-family: ui-monospace, Menlo, monospace; width: 100%; }
input[type=number] { width: 90px; }
button {
background: #22302b; color: var(--fg); border: 1px solid var(--edge); border-radius: 6px;
padding: 8px 12px; font: inherit; cursor: pointer;
}
button:hover { border-color: var(--accent); }
button.primary { background: var(--accent); color: #062019; border-color: var(--accent); font-weight: 600; }
button.danger:hover { border-color: var(--err); color: var(--err); }
.seg { display: inline-flex; border: 1px solid var(--edge); border-radius: 6px; overflow: hidden; }
.seg button { border: 0; border-radius: 0; padding: 7px 11px; background: transparent; }
.seg button.on { background: #22302b; color: var(--accent); }
label.check { display: inline-flex; gap: 6px; align-items: center; color: var(--muted); }
.line { display: grid; gap: 8px; padding: 10px 0; border-top: 1px solid var(--edge); }
.line:first-of-type { border-top: 0; padding-top: 0; }
.muted { color: var(--muted); font-size: 13px; }
#status { min-height: 1.4em; font-size: 14px; }
#status.err { color: var(--err); }
#status.ok { color: var(--ok); }
.vfd { background: #050807; border: 2px solid #3d4a46; border-radius: 8px; padding: 12px; }
.vfd canvas { width: 100%; height: auto; display: block; }
.tabs { display: flex; flex-wrap: wrap; gap: 6px; }
.tabs button { width: 44px; height: 52px; padding: 4px; display: grid; place-items: center; gap: 2px; font-size: 12px; }
.tabs button.on { border-color: var(--accent); color: var(--accent); }
.tabs canvas { width: 15px; height: 21px; }
.gallery { display: grid; grid-template-columns: repeat(auto-fill, minmax(96px, 1fr)); gap: 8px; margin: 4px 0 14px; }
.gallery button { display: grid; justify-items: center; gap: 4px; padding: 10px 6px; font-size: 13px; }
.gallery canvas { width: 30px; height: 42px; }
.gallery small { color: var(--muted); font-size: 11px; }
.frames { display: flex; flex-wrap: wrap; gap: 12px; }
.frame { display: grid; gap: 4px; justify-items: center; }
.grid { display: grid; grid-template-columns: repeat(5, 22px); gap: 3px; touch-action: none; }
.grid div { width: 22px; height: 22px; border-radius: 50%; background: var(--vfd-off); cursor: pointer; }
.grid div.on { background: var(--vfd); box-shadow: 0 0 6px rgba(67, 243, 196, .5); }
details summary { cursor: pointer; color: var(--muted); }
.pagetabs { display: flex; flex-wrap: wrap; gap: 6px; }
.pagetabs button.on { border-color: var(--accent); color: var(--accent); }
.line.kept > :not(.row:first-child) { opacity: .4; pointer-events: none; }
</style>
</head>
<body>
<main>
<div class="row">
<h1 class="grow">BondiMachine VFD</h1>
<input id="esp" type="text" placeholder="IP del ESP32 (ej. 192.168.1.50)" title="Dirección del ESP32">
<button id="reload">Leer del ESP</button>
</div>
<div id="status"></div>
<div class="vfd"><canvas id="preview" width="640" height="112"></canvas></div>
<section>
<h2>Pantallas</h2>
<p class="muted">La secuencia se repite. Cada pantalla dura lo que diga; con 0 dura hasta que terminen sus pasadas y efectos de una sola vez.</p>
<div class="row">
<div class="pagetabs grow" id="pageTabs"></div>
<div class="seg" id="previewMode" title="Qué muestra la vista previa">
<button data-m="seq" class="on">Ver secuencia</button><button data-m="page">Ver esta pantalla</button>
</div>
</div>
<div class="row" style="margin:10px 0">
<label class="check">dura <input type="number" id="pageMs" min="0" max="60000" step="100"> ms</label>
<button id="pageAdd">+ Pantalla</button>
<button id="pageDup">Duplicar</button>
<button id="pageLeft" title="Mover antes">◀</button>
<button id="pageRight" title="Mover después">▶</button>
<button class="danger" id="pageDel">Borrar</button>
</div>
<div id="lines"></div>
<div class="row" style="margin-top:10px">
<button class="primary" id="sendAll">Enviar la secuencia</button>
<button id="loadDemo">Cargar demo</button>
<span class="muted">{0}…{9} insertan caracteres custom, {{ escribe una llave.</span>
</div>
</section>
<section>
<h2>Caracteres custom</h2>
<p class="muted">Galería: tocá una animación para cargarla en el carácter que estás editando.</p>
<div class="gallery" id="gallery"></div>
<div class="tabs" id="tabs"></div>
<p class="muted" id="slots"></p>
<div class="frames" id="frames"></div>
<div class="row" style="margin-top:12px">
<button id="addFrame">+ frame</button>
<button id="delFrame">− frame</button>
<label class="check">cambia cada <input type="number" id="frameMs" min="30" max="10000" step="10" value="250"> ms</label>
</div>
<div class="row" style="margin-top:12px">
<button class="primary" id="saveGlyph">Guardar en el display</button>
<button id="insert1">Insertar en línea 1</button>
<button id="insert2">Insertar en línea 2</button>
<button class="danger" id="deleteGlyph">Borrar</button>
</div>
</section>
<section>
<details>
<summary>WiFi del ESP32</summary>
<p class="muted" id="wifiInfo"></p>
<div class="row">
<input id="ssid" type="text" placeholder="Red" class="grow">
<input id="pass" type="password" placeholder="Clave" class="grow">
<button id="saveWifi">Guardar y reiniciar</button>
</div>
</details>
</section>
</main>
<script>
const COLS = 20, SLOTS = 10, GAP = 4;
const ALIGNS = [["left", "Izq."], ["center", "Centro"], ["right", "Der."]];
/* Galeria de animaciones. Cada frame: 7 filas de 5 columnas ("#" = punto
* encendido). El display es monocromo: los "cambios de color" de los
* fantasmas son, como en el juego, pasar de normal a asustado y titilar. */
const art = (...rows) => rows.map((r) => parseInt(r.replace(/#/g, "1").replace(/\./g, "0"), 2));
const PAC_OPEN = art(".###.", "####.", "###..", "##...", "###..", "####.", ".###.");
const PAC_HALF = art(".###.", "#####", "####.", "##...", "####.", "#####", ".###.");
const PAC_SHUT = art(".###.", "#####", "#####", "#####", "#####", "#####", ".###.");
const mirror = (f) => f.map((r) => parseInt(r.toString(2).padStart(5, "0").split("").reverse().join(""), 2));
const GHOST_A = art(".###.", "#####", "#.#.#", "#####", "#####", "#####", "#.#.#");
const GHOST_B = art(".###.", "#####", "#.#.#", "#####", "#####", "#####", ".#.#.");
const SCARED_A = art(".###.", "#...#", "##.##", "#...#", "#.#.#", "##.##", "#.#.#");
const SCARED_B = art(".###.", "#...#", "##.##", "#...#", "##.##", "#.#.#", ".#.#.");
const EMPTY = art(".....", ".....", ".....", ".....", ".....", ".....", ".....");
const PRESETS = [
{ name: "Pac-Man →", ms: 250, frames: [PAC_OPEN, PAC_SHUT] },
{ name: "Pac-Man ←", ms: 250, frames: [mirror(PAC_OPEN), PAC_SHUT] },
{ name: "Pac-Man suave", ms: 120, frames: [PAC_OPEN, PAC_HALF, PAC_SHUT, PAC_HALF] },
{ name: "Fantasma", ms: 300, frames: [GHOST_A, GHOST_B] },
{ name: "Fantasma asustado", ms: 300, frames: [SCARED_A, SCARED_B] },
{ name: "Fantasma: se asusta", ms: 400, frames: [GHOST_A, SCARED_A] },
{ name: "Fantasma: titila", ms: 180, frames: [SCARED_A, EMPTY] },
{ name: "Pastilla de poder", ms: 300, frames: [art(".....", ".....", ".###.", ".###.", ".###.", ".....", "....."), EMPTY] },
{ name: "Corazón que late", ms: 400, frames: [
art(".....", ".#.#.", "#####", "#####", ".###.", "..#..", "....."),
art(".....", ".....", ".#.#.", ".###.", "..#..", ".....", ".....")] },
{ name: "Invasor", ms: 500, frames: [
art("#...#", ".#.#.", "#####", "#.#.#", "#####", ".#.#.", "#...#"),
art(".#.#.", "#...#", "#####", "#.#.#", "#####", "#...#", ".#.#.")] },
{ name: "Moneda que gira", ms: 180, frames: [
art(".###.", "#...#", "#.#.#", "#.#.#", "#.#.#", "#...#", ".###."),
art("..#..", ".#.#.", ".#.#.", ".#.#.", ".#.#.", ".#.#.", "..#.."),
art("..#..", "..#..", "..#..", "..#..", "..#..", "..#..", "..#.."),
art("..#..", ".#.#.", ".#.#.", ".#.#.", ".#.#.", ".#.#.", "..#..")] },
{ name: "Estrella", ms: 350, frames: [
art("..#..", "..#..", "#####", ".###.", ".#.#.", "#...#", "....."),
art(".....", ".....", "..#..", ".###.", "..#..", ".....", ".....")] },
{ name: "Flecha →", ms: 200, frames: [
art("..#..", "...#.", "#####", "...#.", "..#..", ".....", "....."),
art(".....", "..#..", "...#.", "#####", "...#.", "..#..", ".....")] },
{ name: "Nota musical", ms: 300, frames: [
art("..###", "..#.#", "..#.#", "..#..", "###..", "###..", "....."),
art(".....", "..###", "..#.#", "..#.#", "..#..", "###..", "###..")] },
];
const $ = (s) => document.querySelector(s);
const servedByEsp = location.protocol.startsWith("http");
let pages = [mkPage()];
let cur = 0;                                                                  // pantalla que se edita
let previewMode = "seq";
let glyphs = Array.from({ length: 10 }, () => ({ ms: 250, frames: [] }));   // lo guardado en el ESP
let edit = { id: 0, ms: 250, frames: [blank()] };                             // lo que se esta editando
const SCROLLS = [["static", "Fija"], ["left", "Marquesina ←"], ["right", "Marquesina →"],
["pass-left", "Pasa una vez ←"], ["pass-right", "Pasa una vez →"]];
function mkLine() { return { keep: false, text: "", align: "left", scroll: "static", speed: 200, effect: "none", fxms: 120, cursor: "block" }; }
function mkPage() { return { ms: 5000, lines: [mkLine(), mkLine()] }; }
const clone = (x) => JSON.parse(JSON.stringify(x));
function blank() { return [0, 0, 0, 0, 0, 0, 0]; }
/* ---- Conexion ---------------------------------------------------------- */
function store(k, v) { try { localStorage.setItem(k, v); } catch (e) {} }
function load(k) { try { return localStorage.getItem(k); } catch (e) { return null; } }
function base() {
const v = $("#esp").value.trim().replace(/\/+$/, "");
if (!v) return servedByEsp ? "" : null;
return /^https?:\/\//.test(v) ? v : "http://" + v;
}
async function api(path, params) {
const b = base();
if (b === null) throw new Error("Escribí la IP del ESP32 arriba");
const opts = params ? { method: "POST", body: new URLSearchParams(params) } : {};
let r;
try { r = await fetch(b + path, opts); }
catch (e) { throw new Error("No se pudo conectar con el ESP32 (" + (b || location.host) + ")"); }
const j = await r.json().catch(() => ({}));
if (!r.ok) throw new Error(j.error || "Error " + r.status);
return j;
}
function status(msg, kind) { const s = $("#status"); s.textContent = msg; s.className = kind || ""; }
async function run(label, fn) {
try { const j = await fn(); status(label, "ok"); return j; }
catch (e) { status(e.message, "err"); }
}
/* Toma del ESP la configuracion. sel: null = todas las pantallas, [] = solo
* los caracteres custom, {page, lines} = esas lineas de esa pantalla (lo
* demas puede tener cambios sin enviar). */
function applyState(j, sel = null) {
const fromEsp = (l) => ({ keep: !!l.keep, text: l.text, align: l.align, scroll: l.scroll, speed: l.speed || 200,
effect: l.effect || "none", fxms: l.fxms || 120, cursor: l.cursor || "block" });
if (sel === null) {
pages = j.pages.map((p) => ({ ms: p.ms, lines: p.lines.map(fromEsp) }));
if (!pages.length) pages = [mkPage()];
cur = Math.min(cur, pages.length - 1);
} else if (sel.lines) {
const p = j.pages[sel.page];
if (!pages[sel.page]) pages[sel.page] = mkPage();
pages[sel.page].ms = p.ms;
sel.lines.forEach((i) => { pages[sel.page].lines[i] = fromEsp(p.lines[i]); });
}
glyphs = Array.from({ length: 10 }, () => ({ ms: 250, frames: [] }));
j.glyphs.forEach((g) => {
glyphs[g.id] = { ms: g.ms, frames: g.frames.map((h) => h.match(/../g).map((x) => parseInt(x, 16))) };
});
if (j.wifi) $("#wifiInfo").textContent = (j.wifi.mode === "ap" ? "Red propia " : "Conectado a ") + j.wifi.ssid + " · " + j.wifi.ip;
if (sel === null || sel.lines) renderPages();
else renderTabs();
resetAnim();
}
async function refresh() {
const j = await run("Leído del ESP32", () => api("/api/state"));
if (j) { applyState(j); selectGlyph(edit.id); }
}
/* ---- Pantallas y lineas ------------------------------------------------ */
function renderPages() {
const box = $("#pageTabs");
box.innerHTML = "";
pages.forEach((p, i) => {
const b = document.createElement("button");
b.textContent = "Pantalla " + (i + 1);
b.className = i === cur ? "on" : "";
b.addEventListener("click", () => { cur = i; renderPages(); });
box.appendChild(b);
});
$("#pageMs").value = pages[cur].ms;
renderLines();
resetAnim();
}
function renderLines() {
const box = $("#lines");
box.innerHTML = "";
pages[cur].lines.forEach((l, i) => {
const d = document.createElement("div");
d.className = "line" + (l.keep ? " kept" : "");
d.innerHTML = `
<div class="row"><strong>Línea ${i + 1}</strong>
<label class="check grow"><input type="checkbox" class="keep" ${l.keep ? "checked" : ""}> igual que en la pantalla anterior</label>
<div class="seg">${ALIGNS.map(([v, t]) => `<button data-a="${v}" class="${l.align === v ? "on" : ""}">${t}</button>`).join("")}</div>
</div>
<input type="text" class="mono" maxlength="80" spellcheck="false">
<div class="row">
<select class="scroll">${SCROLLS.map(([v, t]) => `<option value="${v}">${t}</option>`).join("")}</select>
<label class="check">paso cada <input type="number" class="speed" min="20" max="5000" step="10" value="${l.speed}"> ms</label>
</div>
<div class="row fx">
<select class="effect" title="Efecto (solo en líneas fijas)">
<option value="none">Sin efecto</option>
<option value="type">Tipear</option>
<option value="erase">Borrar</option>
<option value="cycle">Tipear y borrar (loop)</option>
</select>
<label class="check">cada <input type="number" class="fxms" min="20" max="5000" step="10" value="${l.fxms}"> ms</label>
<label class="check">cursor
<select class="cursor">
<option value="block">Bloque que titila</option>
<option value="none">Sin cursor</option>
${Array.from({ length: 10 }, (_, k) => `<option value="${k}">{${k}}</option>`).join("")}
</select>
</label>
<span class="grow"></span>
<button class="send">Enviar línea ${i + 1}</button>
</div>`;
const $$ = (q) => d.querySelector(q);
const text = $$("input[type=text]");
$$(".scroll").value = l.scroll;
$$(".effect").value = l.effect;
$$(".cursor").value = l.cursor;
const fxState = () => { $$(".fx").style.opacity = l.scroll !== "static" ? 0.45 : ""; };
fxState();
text.value = l.text;
text.addEventListener("input", () => { l.text = text.value; resetAnim(); });
$$(".keep").addEventListener("change", (e) => { l.keep = e.target.checked; d.classList.toggle("kept", l.keep); resetAnim(); });
$$(".scroll").addEventListener("change", (e) => { l.scroll = e.target.value; fxState(); resetAnim(); });
$$(".speed").addEventListener("input", (e) => { l.speed = +e.target.value || 200; resetAnim(); });
$$(".effect").addEventListener("change", (e) => { l.effect = e.target.value; resetAnim(); });
$$(".fxms").addEventListener("input", (e) => { l.fxms = +e.target.value || 120; resetAnim(); });
$$(".cursor").addEventListener("change", (e) => { l.cursor = e.target.value; resetAnim(); });
d.querySelectorAll(".seg button").forEach((b) => b.addEventListener("click", () => {
l.align = b.dataset.a;
d.querySelectorAll(".seg button").forEach((x) => x.classList.toggle("on", x === b));
resetAnim();
}));
$$(".send").addEventListener("click", () => sendLine(i));
box.appendChild(d);
});
}
function lineParams(l, n, pre) {
return { [pre + "keep" + n]: l.keep ? 1 : 0, [pre + "text" + n]: l.text, [pre + "align" + n]: l.align,
[pre + "scroll" + n]: l.scroll, [pre + "speed" + n]: l.speed, [pre + "effect" + n]: l.effect,
[pre + "fxms" + n]: l.fxms, [pre + "cursor" + n]: l.cursor };
}
async function sendLine(i) {
const params = { page: cur, ms: pages[cur].ms, ...lineParams(pages[cur].lines[i], i + 1, "") };
const j = await run("Línea " + (i + 1) + " de la pantalla " + (cur + 1) + " enviada", () => api("/api/lines", params));
if (j) applyState(j, { page: cur, lines: [i] });
}
async function sendAll() {
const params = { n: pages.length };
pages.forEach((p, k) => {
params[`p${k}_ms`] = p.ms;
p.lines.forEach((l, i) => Object.assign(params, lineParams(l, i + 1, `p${k}_`)));
});
const j = await run("Secuencia enviada (" + pages.length + (pages.length > 1 ? " pantallas)" : " pantalla)"),
() => api("/api/pages", params));
if (j) applyState(j);
}
/* ---- Vista previa (misma logica que screen.c) --------------------------- */
/* charset:begin - generado por charset.py */
const FONT = {"20":"00000000000000","21":"04040404040004","22":"0A0A0A00000000","23":"0A0A1F0A1F0A0A","24":"040F140E051E04","25":"18190204081303","26":"0814140815120D","27":"04040400000000","28":"04081010100804","29":"04020101010204","2A":"04150E040E1504","2B":"0004041F040400","2C":"00000000040408","2D":"0000001F000000","2E":"00000000000004","2F":"01010204081010","30":"0E11131519110E","31":"040C040404040E","32":"0E11010E10101F","33":"0E11010601110E","34":"02060A121F0202","35":"1F101E0101110E","36":"0608101E11110E","37":"1F010204080808","38":"0E11110E11110E","39":"0E11110F01020C","3A":"00000400040000","3B":"00000400040408","3C":"02040810080402","3D":"00001F001F0000","3E":"08040201020408","3F":"0E110204040004","40":"0E11171517100E","41":"040A11111F1111","42":"1E09090E09091E","43":"0E11101010110E","44":"1E09090909091E","45":"1F10101E10101F","46":"1F10101E101010","47":"0E11101013110F","48":"1111111F111111","49":"0E04040404040E","4A":"0101010101110E","4B":"11121418141211","4C":"1010101010101F","4D":"111B1511111111","4E":"11111915131111","4F":"0E11111111110E","50":"1E11111E101010","51":"0E11111115120D","52":"1E11111E141211","53":"0E11100E01110E","54":"1F040404040404","55":"1111111111110E","56":"1111110A0A0404","57":"11111115151B11","58":"11110A040A1111","59":"11110A04040404","5A":"1F01020408101F","5B":"1F18181818181F","5C":"110A1F041F0404","5D":"1F03030303031F","5E":"0000040A110000","5F":"0000000000001F","60":"00100804000000","61":"00000E010F110F","62":"1010101E11111E","63":"00000F1010100F","64":"0101010F11110F","65":"00000E111E100E","66":"0205040E040404","67":"00000E110F010E","68":"1010101E111111","69":"00040004040404","6A":"01000101010906","6B":"0808090A0C0A09","6C":"0C04040404040E","6D":"00001B15151511","6E":"00001619111111","6F":"00000E1111110E","70":"00001E111E1010","71":"00000F110F0101","72":"00001619101010","73":"00000F100E011E","74":"04041F04040404","75":"0000111111110E","76":"00001111110A04","77":"0000111115150A","78":"0000110A040A11","79":"0000110A040408","7A":"00001F0204081F","7B":"06080810080806","7C":"0609081C08081F","7D":"0C02020102020C","7E":"1C141C00000000","7F":"150A150A150A15","A0":"0A000E111F1111","A1":"04000E111F1111","A2":"02040E111F1111","A3":"0F1010100F0408","A4":"040A0E111F1111","A5":"070C141F141417","A6":"02041F101E101F","A7":"08041F101E101F","A8":"040A1F101E101F","A9":"0A000E0404040E","AA":"02040E0404040E","AB":"08040E0404040E","AC":"040A0E0404040E","AD":"0A000E1111110E","AE":"0D160019151311","AF":"0E11131519110E","B0":"02040E1111110E","B1":"08040E1111110E","B2":"08040E111F1111","B3":"1E111112111112","B4":"0A001F101E101F","B5":"0A00111111110E","B6":"0204111111110E","B7":"0804111111110E","B8":"040A001111110E","B9":"040A0400000000","BA":"04041F04041F00","BB":"0004001F000400","BC":"00041E1F1E0400","BD":"00041E111E0400","BE":"00040E1F0E0400","BF":"040A0E1111110E","C0":"0A000E010F110F","C1":"04000E010F110F","C2":"02040E010F110F","C3":"000E10100E0408","C4":"040A0E010F110F","C5":"00001A051F140B","C6":"02040E111F100F","C7":"08040E111F100F","C8":"040A0E111F100F","C9":"000A0004040404","CA":"02040004040404","CB":"08040004040404","CC":"040A0004040404","CD":"000A000E0A0A0E","CE":"0D160016191111","CF":"00000E1315190E","D0":"0204000E0A0A0E","D1":"0804000E0A0A0E","D2":"08040E010F110F","D3":"00040E14140E04","D4":"0A000E111F100F","D5":"000A000A0A0A0E","D6":"0204000A0A0A0E","D7":"0804000A0A0A0E","D8":"040A000A0A0A0E","D9":"000E1111110E00","DA":"000E1115110E00","DB":"000E1F1F1F0E00","DC":"001F1111111F00","DD":"001F1F1F1F1F00","DE":"00040A110A0400","DF":"040A000E0A0A0E","E0":"1F11151515111F","E1":"1F1B131B1B111F","E2":"1F131D1B17111F","E3":"1F131D1B1D131F","E4":"1F1D1915111D1F","E5":"1F1117131D131F","E6":"1F17171115111F","E7":"1F111D1B17171F","E8":"1F11151115111F","E9":"1F1115111D1D1F","EA":"00000000000000","EB":"00000000000000","EC":"00000000000000","ED":"00000000000000","EE":"00000000000000","EF":"1F1F1F1F1F1F1F"};
const UNI = {"¥":92,"£":124,"Ä":160,"Å":161,"Á":162,"Ç":163,"Â":164,"Æ":165,"É":166,"È":167,"Ê":168,"Ï":169,"Í":170,"Ì":171,"Î":172,"Ö":173,"Ñ":174,"Ø":175,"Ó":176,"Ò":177,"À":178,"ß":179,"Ë":180,"Ü":181,"Ú":182,"Ù":183,"Û":184,"°":185,"±":186,"÷":187,"→":188,"⇒":189,"◆":190,"Ô":191,"ä":192,"å":193,"á":194,"ç":195,"â":196,"æ":197,"é":198,"è":199,"ê":200,"ï":201,"í":202,"ì":203,"î":204,"ö":205,"ñ":206,"ø":207,"ó":208,"ò":209,"à":210,"¢":211,"ë":212,"ü":213,"ú":214,"ù":215,"û":216,"○":217,"◉":218,"●":219,"□":220,"■":221,"◇":222,"ô":223,"█":239};
/* charset:end */
const FONT_ROWS = {};
for (const [k, v] of Object.entries(FONT)) FONT_ROWS[parseInt(k, 16)] = v.match(/../g).map((x) => parseInt(x, 16));
const SPACE = { c: 0x20 };
/* Caracter de la pagina -> codigo del display (igual que screen.c) */
function charCode(ch) {
if (UNI[ch] !== undefined) return UNI[ch];
const u = ch.codePointAt(0);
if (u >= 0x20 && u < 0x7F) return u;
return u < 0x20 || u === 0x7F ? 0x20 : 0x3F;
}
/* {c: codigo} o {g: caracter custom} */
function tokens(text) {
const t = Array.from(text), out = [];
for (let i = 0; i < t.length; i++) {
if (t[i] === "{" && t[i + 1] === "{") { out.push({ c: 0x7B }); i++; }
else if (t[i] === "{" && /^[0-9]$/.test(t[i + 1] || "") && t[i + 2] === "}") { out.push({ g: +t[i + 1] }); i += 2; }
else out.push({ c: charCode(t[i]) });
}
return out.slice(0, 80);
}
let anim = { t0: performance.now() };
function resetAnim() { anim.t0 = performance.now(); }
/* Efectos, con los mismos tiempos que screen.c */
const HOLD_MS = 2000, GAP_MS = 1000, BLINK_MS = 300, BLOCK = { c: 0xEF };
function effectState(l, n, dt) {
if (l.effect === "none") return null;
const steps = (n + 1) * l.fxms;          // n pasos + uno con el cursor al final
let phase, t = dt;
if (l.effect === "type") phase = t < steps ? "typing" : "full";
else if (l.effect === "erase") phase = t < steps ? "erasing" : "empty";
else {
t = dt % (2 * steps + HOLD_MS + GAP_MS);
if (t < steps) phase = "typing";
else if ((t -= steps) < HOLD_MS) phase = "full";
else if ((t -= HOLD_MS) < steps) phase = "erasing";
else phase = "empty";
}
const pos = Math.min(Math.floor(t / l.fxms), n);
let cursor = null;
if ((phase === "typing" || phase === "erasing") && l.cursor !== "none") {
if (l.cursor === "block") cursor = Math.floor(dt / BLINK_MS) % 2 ? SPACE : BLOCK;
else cursor = { g: +l.cursor };
}
return { phase, pos, cursor };
}
/* Lo que muestra una linea dt ms despues de que se puso (como screen.c) */
function cells(l, dt) {
const tk = tokens(l.text), n = tk.length, out = Array(COLS).fill(SPACE);
const start = n >= COLS ? 0 : l.align === "center" ? Math.floor((COLS - n) / 2) : l.align === "right" ? COLS - n : 0;
const fx = l.scroll === "static" ? effectState(l, n, dt) : null;
const step = Math.floor(dt / l.speed), period = n + (n < COLS - GAP ? COLS - n : GAP);
const pass = Math.min(step, COLS + n);
for (let k = 0; k < n; k++) {
if (fx && (fx.phase === "empty" || (fx.phase === "typing" && k >= fx.pos) || (fx.phase === "erasing" && k < fx.pos))) continue;
let col;
switch (l.scroll) {
case "left": col = ((k - step) % period + period) % period; break;
case "right": col = (k + step) % period; break;
case "pass-left": col = COLS - pass + k; break;
case "pass-right": col = pass - n + k; break;
default: col = start + k;
}
if (col >= 0 && col < COLS) out[col] = tk[k];
}
if (fx && fx.cursor && start + fx.pos < COLS) out[start + fx.pos] = fx.cursor;
return out;
}
/* Cuanto tarda una linea en quedar quieta: 0 si ya lo esta, Infinity si nunca */
function lineDur(l) {
const n = tokens(l.text).length;
if (l.scroll === "pass-left" || l.scroll === "pass-right") return (COLS + n) * l.speed;
if (l.scroll !== "static") return Infinity;
if (l.effect === "type" || l.effect === "erase") return (n + 1) * l.fxms;
return l.effect === "cycle" ? Infinity : 0;
}
/* Pantalla donde se puso la linea i que se ve en la pantalla p */
function source(p, i) {
let q = p, back = 0;
while (pages[q].lines[i].keep && back < pages.length) { q = (q - 1 + pages.length) % pages.length; back++; }
return { q, back };
}
function pageDur(p) {
if (pages[p].ms) return pages[p].ms;
let d = 50;                                    // al menos lo que tarda en dibujarse
for (let i = 0; i < 2; i++) {
const { q, back } = source(p, i), ld = lineDur(pages[q].lines[i]);
if (ld === Infinity) return Infinity;
if (!back) d = Math.max(d, ld);
}
return d;
}
/* Que pantalla se ve y desde cuando esta cada linea, t ms despues de empezar */
function previewAt(t) {
if (previewMode === "page" || pages.length === 1) {
const p = previewMode === "page" ? cur : 0;
return { p, dts: [0, 1].map((i) => t), src: [0, 1].map((i) => source(p, i).q) };
}
const durs = pages.map((_, p) => pageDur(p));
let tt = t, total = durs.reduce((a, b) => a + b, 0);
if (total !== Infinity) tt = t % total;
let p = 0, acc = 0;
while (p < pages.length - 1 && acc + durs[p] <= tt) acc += durs[p++];
const pageStart = t - (tt - acc);
const dts = [], src = [];
for (let i = 0; i < 2; i++) {
const { q, back } = source(p, i);
let s = pageStart;
for (let k = 1; k <= back; k++) s -= durs[(p - k + pages.length) % pages.length];
dts.push(t - s); src.push(q);
}
return { p, dts, src };
}
function glyphFor(id) {
const g = id === edit.id ? edit : glyphs[id];
return g.frames.length ? g : null;
}
const cv = $("#preview"), ctx = cv.getContext("2d");
const CW = 32, CH = 56, DOT = 5, PITCH = 6;
function drawDots(c, x0, y0, rows, on, off) {
for (let r = 0; r < 7; r++) for (let k = 0; k < 5; k++) {
c.fillStyle = rows && rows[r] & (16 >> k) ? on : off;
c.fillRect(x0 + k * PITCH, y0 + r * PITCH, DOT, DOT);
}
}
function drawPreview() {
const now = performance.now();
ctx.fillStyle = "#050807"; ctx.fillRect(0, 0, cv.width, cv.height);
const st = previewAt(now - anim.t0);
[0, 1].map((li) => cells(pages[st.src[li]].lines[li], st.dts[li])).forEach((row, li) => row.forEach((c, ci) => {
const x = ci * CW + 2, y = li * CH + 6;
let rows = FONT_ROWS[c.c];
if (c.g !== undefined) {
const g = glyphFor(c.g);
rows = g ? g.frames[g.frames.length > 1 ? Math.floor(now / g.ms) % g.frames.length : 0] : null;
}
drawDots(ctx, x, y, rows, "#43f3c4", "#15241f");
}));
drawGallery(now);
requestAnimationFrame(drawPreview);
}
/* ---- Caracteres custom ------------------------------------------------- */
/* Frames distintos: los iguales comparten ubicacion del display */
function slotsUsed() {
const set = new Set();
glyphs.forEach((g, i) => (i === edit.id ? edit : g).frames.forEach((f) => set.add(f.map((v) => v & 31).join())));
return set.size;
}
function renderTabs() {
const box = $("#tabs");
box.innerHTML = "";
for (let i = 0; i < 10; i++) {
const b = document.createElement("button");
b.className = i === edit.id ? "on" : "";
const c = document.createElement("canvas");
c.width = 30; c.height = 42;
const g = i === edit.id ? edit : glyphs[i];
const x = c.getContext("2d");
for (let r = 0; r < 7; r++) for (let k = 0; k < 5; k++) {
x.fillStyle = g.frames[0] && g.frames[0][r] & (16 >> k) ? "#43f3c4" : "#15241f";
x.fillRect(k * 6, r * 6, 5, 5);
}
b.append(c, document.createTextNode("{" + i + "}"));
b.addEventListener("click", () => selectGlyph(i));
box.appendChild(b);
}
const used = slotsUsed();
$("#slots").textContent = `Frames distintos en uso: ${used} de ${SLOTS} (cada uno ocupa un carácter custom del display; los iguales se comparten).`;
$("#slots").style.color = used > SLOTS ? "var(--err)" : "";
}
let painting = null;
function renderFrames() {
const box = $("#frames");
box.innerHTML = "";
edit.frames.forEach((rows, fi) => {
const f = document.createElement("div");
f.className = "frame";
const g = document.createElement("div");
g.className = "grid";
for (let r = 0; r < 7; r++) for (let k = 0; k < 5; k++) {
const d = document.createElement("div");
const bit = 16 >> k;
d.className = rows[r] & bit ? "on" : "";
const set = (v) => { rows[r] = v ? rows[r] | bit : rows[r] & ~bit; d.className = v ? "on" : ""; renderTabs(); };
d.addEventListener("pointerdown", (e) => { e.preventDefault(); painting = !(rows[r] & bit); set(painting); });
d.addEventListener("pointerenter", () => { if (painting !== null) set(painting); });
g.appendChild(d);
}
const t = document.createElement("span");
t.className = "muted";
t.textContent = "frame " + (fi + 1);
f.append(g, t);
box.appendChild(f);
});
$("#frameMs").value = edit.ms;
renderTabs();
}
addEventListener("pointerup", () => { painting = null; });
function selectGlyph(i) {
const g = glyphs[i];
edit = { id: i, ms: g.ms || 250, frames: g.frames.length ? g.frames.map((f) => f.slice()) : [blank()] };
renderFrames();
}
function hexFrames(frames) {
return frames.map((f) => f.map((v) => v.toString(16).padStart(2, "0").toUpperCase()).join("")).join(",");
}
async function saveGlyph(frames) {
const j = await run(frames.length ? "Carácter {" + edit.id + "} guardado" : "Carácter {" + edit.id + "} borrado",
() => api("/api/glyph", { id: edit.id, frames: hexFrames(frames), ms: edit.ms }));
if (j) applyState(j, []);
}
function insertToken(i) {
const l = pages[cur].lines[i];
const input = $("#lines").querySelectorAll("input[type=text]")[i];
const pos = input.selectionStart ?? l.text.length;
l.text = l.text.slice(0, pos) + "{" + edit.id + "}" + l.text.slice(input.selectionEnd ?? pos);
input.value = l.text;
resetAnim();
}
/* ---- Galeria ----------------------------------------------------------- */
const galleryCanvases = [];
function renderGallery() {
const box = $("#gallery");
PRESETS.forEach((p) => {
const b = document.createElement("button");
const c = document.createElement("canvas");
c.width = 30; c.height = 42;
const n = document.createElement("small");
n.textContent = p.frames.length + (p.frames.length > 1 ? " frames" : " frame");
b.append(c, document.createTextNode(p.name), n);
b.title = "Cargar en el carácter que se está editando";
b.addEventListener("click", () => {
edit.ms = p.ms;
edit.frames = p.frames.map((f) => f.slice());
renderFrames();
status(p.name + " cargado en {" + edit.id + "}: tocá Guardar en el display para mandarlo", "ok");
});
box.appendChild(b);
galleryCanvases.push([c.getContext("2d"), p]);
});
}
function drawGallery(now) {
for (const [c, p] of galleryCanvases) {
const f = p.frames[Math.floor(now / p.ms) % p.frames.length];
for (let r = 0; r < 7; r++) for (let k = 0; k < 5; k++) {
c.fillStyle = f[r] & (16 >> k) ? "#43f3c4" : "#15241f";
c.fillRect(k * 6, r * 6, 5, 5);
}
}
}
/* ---- Eventos ----------------------------------------------------------- */
$("#esp").value = servedByEsp ? "" : load("bondi.esp") || "";
$("#esp").placeholder = servedByEsp ? location.host + " (este ESP32)" : "IP del ESP32 (ej. 192.168.1.50)";
$("#esp").addEventListener("change", () => { store("bondi.esp", $("#esp").value.trim()); refresh(); });
$("#reload").addEventListener("click", refresh);
$("#sendAll").addEventListener("click", sendAll);
$("#loadDemo").addEventListener("click", async () => {
const j = await run("Demo cargada", () => api("/api/demo", {}));
if (j) { applyState(j); selectGlyph(edit.id); }
});
$("#pageMs").addEventListener("input", (e) => { pages[cur].ms = Math.max(0, +e.target.value || 0); resetAnim(); });
$("#pageAdd").addEventListener("click", () => { pages.splice(cur + 1, 0, mkPage()); cur++; renderPages(); });
$("#pageDup").addEventListener("click", () => { pages.splice(cur + 1, 0, clone(pages[cur])); cur++; renderPages(); });
$("#pageDel").addEventListener("click", () => {
if (pages.length > 1) { pages.splice(cur, 1); cur = Math.min(cur, pages.length - 1); renderPages(); }
});
const movePage = (d) => {
const to = cur + d;
if (to < 0 || to >= pages.length) return;
[pages[cur], pages[to]] = [pages[to], pages[cur]];
cur = to;
renderPages();
};
$("#pageLeft").addEventListener("click", () => movePage(-1));
$("#pageRight").addEventListener("click", () => movePage(1));
$("#previewMode").querySelectorAll("button").forEach((b) => b.addEventListener("click", () => {
previewMode = b.dataset.m;
$("#previewMode").querySelectorAll("button").forEach((x) => x.classList.toggle("on", x === b));
resetAnim();
}));
$("#addFrame").addEventListener("click", () => {
edit.frames.push((edit.frames[edit.frames.length - 1] || blank()).slice());
renderFrames();
});
$("#delFrame").addEventListener("click", () => { if (edit.frames.length > 1) edit.frames.pop(); renderFrames(); });
$("#frameMs").addEventListener("input", () => { edit.ms = +$("#frameMs").value || 250; });
$("#saveGlyph").addEventListener("click", () => saveGlyph(edit.frames));
$("#deleteGlyph").addEventListener("click", () => { edit.frames = [blank()]; renderFrames(); saveGlyph([]); });
$("#insert1").addEventListener("click", () => insertToken(0));
$("#insert2").addEventListener("click", () => insertToken(1));
$("#saveWifi").addEventListener("click", async () => {
const ssid = $("#ssid").value.trim();
if (!ssid) return status("Falta el nombre de la red", "err");
const ok = await run("Guardado. El ESP32 se reinicia y muestra su nueva IP en el display.",
() => api("/api/wifi", { ssid, pass: $("#pass").value }));
if (ok) $("#pass").value = "";
});
renderPages();
renderGallery();
selectGlyph(0);
drawPreview();
if (base() !== null) refresh();
else status("Escribí la IP del ESP32 arriba (la muestra el display al arrancar).");
</script>
</body>
</html>)html";
