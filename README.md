# Radar WiFi real con ESP32 — v1 funcional

Esto NO es una simulación: el ESP32 captura los datos finos del WiFi (CSI)
de tu propio router y sirve una página web con la gráfica en vivo. La abrís
desde el celular, en el mismo WiFi, sin instalar ninguna app.

## Qué vas a ver

- La gráfica en vivo de las amplitudes CSI (una barra por subportadora).
- El RSSI y la cantidad de muestras.
- Botón «Calibrar cuarto vacío»: guarda la señal del cuarto sin nadie.
- Después, al caminar entre el router y el ESP32, la distorsión sube y salta
  «¡Movimiento detectado!» con su registro.

## Qué NO hace esta v1 (para no venderte humo)

- No dibuja figuras humanas ni posiciones en un mapa. Eso es lo del video
  viral; la v1 real muestra la señal y detecta movimiento.
- No mide si no hay tráfico WiFi en la casa: el ESP32 captura CSI de los
  paquetes que recibe de tu router (beacons y tráfico normal). Más tráfico
  (un video en el celular, por ejemplo) = más muestras por segundo.

## Qué necesitas

- 1 placa ESP32 DevKit V1 clásica (ESP-WROOM-32) + cable USB.
- Una PC con Arduino IDE.
- Tu WiFi de 2.4 GHz (el ESP32 clásico solo usa 2.4 GHz, no 5 GHz).

## Pasos

1. En Arduino IDE, instala el paquete de placas ESP32 (en Preferencias,
   URL adicional del gestor de tarjetas: la oficial de Espressif) y elegí
   la placa «ESP32 Dev Board» y el puerto USB.
2. Abrí `esp32_radar_web.ino` y completá arriba del todo tu SSID y
   contraseña del WiFi 2.4 GHz.
3. Subí el programa a la placa.
4. Abrí el Monitor Serie en 115200. Ahí vas a ver la IP del ESP32 y el
   contador de muestras CSI (si el contador no sube, falta tráfico WiFi).
5. En el celular, conectado al mismo WiFi, abrí `http://<IP-del-ESP32>/`.
6. Dejá el ESP32 quieto en una mesa, pulsá «Calibrar cuarto vacío» sin
   nadie en el cuarto, y después caminá entre el router y el ESP32.

## Si algo falla

- No conecta al WiFi: revisá SSID y contraseña, y que sea la red 2.4 GHz.
- No suben las muestras: generá tráfico (abrí un video en el celular).
- Arduino da un error al compilar: pasame el texto exacto del error y lo
  corregimos. Este código no se pudo compilar en mi computadora (no tengo
  placa), se compila en la tuya.

## Qué sigue si funciona

Afinar el umbral, guardar un historial, y más adelante probar con más de
un ESP32 para ubicar mejor el movimiento.
