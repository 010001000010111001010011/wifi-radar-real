// Radar WiFi REAL con ESP32 — v1 funcional (NO es simulación)
// El ESP32 captura CSI (los datos finos del WiFi) de tu router y sirve
// una página web con la gráfica en vivo. La abrís desde el celular,
// en el mismo WiFi, sin instalar ninguna app.
// Placa: ESP32 DevKit V1 clásico (ESP-WROOM-32). Solo WiFi 2.4 GHz.
// Basado en la API oficial de Espressif: esp_wifi_set_csi_rx_cb,
// esp_wifi_set_csi_config, esp_wifi_set_csi.

#include <WiFi.h>
#include <WebServer.h>
#include <math.h>
#include <string.h>
#include "esp_wifi.h"

// ---- COMPLETA ESTO CON TU WIFI 2.4 GHz ----
const char* WIFI_SSID = "TU_WIFI_2_4GHZ";
const char* WIFI_PASS = "TU_CONTRASENA";

WebServer server(80);

// Última muestra CSI. La llena el callback (contexto WiFi) y la lee el loop.
// Para una v1 esto alcanza; si más adelante se pierde alguna muestra,
// se cambia por una cola, pero el funcionamiento es el mismo.
#define MAX_CSI_BYTES 256
static int8_t csiBuf[MAX_CSI_BYTES];
static uint16_t csiLen = 0;
static int csiRssi = 0;
static volatile unsigned long csiCount = 0;

void csiCallback(void *ctx, wifi_csi_info_t *data) {
  (void)ctx;
  if (!data || !data->buf || data->len == 0) return;
  uint16_t n = data->len > MAX_CSI_BYTES ? MAX_CSI_BYTES : data->len;
  memcpy(csiBuf, data->buf, n);
  csiLen = n;
  csiRssi = data->rx_ctrl.rssi;
  csiCount++;
}

const char PAGINA[] PROGMEM = R"HTML(<!DOCTYPE html><html lang="es"><head><meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>Radar WiFi real</title>
<style>
body{font-family:sans-serif;background:#0b1220;color:#e5e7eb;margin:0;padding:12px}
h1{font-size:20px;margin:0 0 4px}
p{color:#9ca3af;font-size:14px}
canvas{width:100%;height:160px;background:#111827;border-radius:8px}
button{padding:10px 14px;margin:6px 6px 6px 0;border:0;border-radius:8px;background:#22c55e;color:#04120a;font-weight:bold}
#alerta{padding:10px;border-radius:8px;background:#1f2937;margin-top:8px;font-weight:bold}
#alerta.activa{background:#dc2626;color:#fff}
#log{font-size:13px;color:#9ca3af;white-space:pre-wrap}
</style></head>
<body>
<h1>Radar WiFi real (ESP32)</h1>
<p>Datos CSI reales del ESP32. RSSI: <b id="rssi">-</b> dBm · Muestras: <b id="n">0</b> · Distorsión: <b id="dist">-</b>%</p>
<canvas id="g" width="640" height="160"></canvas>
<div>
<button onclick="calibrar()">Calibrar cuarto vacío</button>
<label>Sensibilidad <input id="umbral" type="range" min="5" max="60" value="20"> <span id="umbralVal">20</span>%</label>
</div>
<div id="alerta">Sin calibrar: pulsa «Calibrar cuarto vacío» sin nadie en el cuarto.</div>
<div id="log"></div>
<script>
var base=null, umbral=20;
document.getElementById('umbral').oninput=function(e){umbral=+e.target.value;document.getElementById('umbralVal').textContent=umbral;};
function calibrar(){fetch('/data').then(function(r){return r.json();}).then(function(d){base=d.amp;document.getElementById('alerta').textContent='Calibrado con '+d.amp.length+' subportadoras. Ahora camina entre el router y el ESP32.';document.getElementById('alerta').className='';});}
function loguear(t){var l=document.getElementById('log');l.textContent=(new Date().toLocaleTimeString())+' - '+t+'\n'+l.textContent;}
var ultimoAlerta=0;
function tick(){
fetch('/data').then(function(r){return r.json();}).then(function(d){
document.getElementById('rssi').textContent=d.rssi;
document.getElementById('n').textContent=d.n;
var cv=document.getElementById('g'),ctx=cv.getContext('2d');
ctx.clearRect(0,0,cv.width,cv.height);
var max=1,i,a;
for(i=0;i<d.amp.length;i++){if(d.amp[i]>max)max=d.amp[i];}
for(i=0;i<d.amp.length;i++){
a=d.amp[i]/max*150;
ctx.fillStyle='#22c55e';
ctx.fillRect(i*(cv.width/d.amp.length),160-a,cv.width/d.amp.length-1,a);
}
if(base&&base.length===d.amp.length){
var suma=0,mb=0;
for(i=0;i<d.amp.length;i++){suma+=Math.abs(d.amp[i]-base[i]);mb+=base[i];}
mb=mb/d.amp.length||1;
var dist=Math.round(suma/d.amp.length/mb*100);
document.getElementById('dist').textContent=dist;
var al=document.getElementById('alerta');
if(dist>umbral){
al.textContent='¡Movimiento detectado! Distorsión '+dist+'%';
al.className='activa';
var ahora=Date.now();
if(ahora-ultimoAlerta>5000){ultimoAlerta=ahora;loguear('Movimiento detectado - distorsión '+dist+'%');}
}else{al.textContent='Sin movimiento (distorsión '+dist+'%)';al.className='';}
}
}).catch(function(){});
}
setInterval(tick,300);tick();
</script></body></html>)HTML";

void handleRoot() {
  server.send_P(200, "text/html", PAGINA);
}

void handleData() {
  // Amplitud por subportadora: sqrt(I^2 + Q^2). El orden I/Q no afecta
  // a la amplitud, así que sirve tal cual venga del chip.
  String j = "{\"rssi\":";
  j += String(csiRssi);
  j += ",\"n\":";
  j += String(csiLen / 2);
  j += ",\"amp\":[";
  for (uint16_t i = 0; i + 1 < csiLen; i += 2) {
    int ii = csiBuf[i];
    int qq = csiBuf[i + 1];
    int amp = (int)sqrt((float)(ii * ii + qq * qq));
    if (i > 0) j += ",";
    j += String(amp);
  }
  j += "]}";
  server.send(200, "application/json", j);
}

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("Radar WiFi real - iniciando...");

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.print("Conectando a WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("IP del ESP32: ");
  Serial.println(WiFi.localIP());
  Serial.println("Abri esa IP desde el celular, en el mismo WiFi.");

  // Configuración CSI (API oficial Espressif)
  wifi_csi_config_t cfg = {};
  cfg.lltf_en = true;
  cfg.htltf_en = true;
  cfg.stbc_htltf2_en = true;
  cfg.ltf_merge_en = true;
  cfg.channel_filter_en = true;
  cfg.manu_scale = false;
  esp_wifi_set_csi_config(&cfg);
  esp_wifi_set_csi_rx_cb(&csiCallback, NULL);
  esp_wifi_set_csi(true);

  server.on("/", handleRoot);
  server.on("/data", handleData);
  server.begin();
  Serial.println("Radar listo.");
}

void loop() {
  server.handleClient();
  static unsigned long t0 = 0;
  if (millis() - t0 > 2000) {
    t0 = millis();
    Serial.print("Muestras CSI: ");
    Serial.print(csiCount);
    Serial.print(" | len=");
    Serial.print(csiLen);
    Serial.print(" | rssi=");
    Serial.println(csiRssi);
  }
}
