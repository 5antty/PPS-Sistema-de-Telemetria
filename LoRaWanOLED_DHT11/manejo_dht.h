#ifndef MANEJO_DHT_H
#define MANEJO_DHT_H

#include <Arduino.h>
#include <DHT.h>

// ─── Configuración del DHT11 ──────────────────────────────────────────────────
#define DHTPIN 13 // GPIO conectado al pin DATA del DHT11
                  // Cambiarlo según cableado (pines libres en V2:
                  // 13, 17, 22, 23, etc.)
#define DHTTYPE DHT11

// Variables globales para los datos del sensor
extern float temperature;
extern float humidity;

void initDHT();
bool readDHT11();

#endif