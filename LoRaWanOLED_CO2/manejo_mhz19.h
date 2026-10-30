#ifndef MANEJO_MHZ19_H
#define MANEJO_MHZ19_H

#include <Arduino.h>

// ─── MH-Z19B (CO2) ─────────────────────────────────────────────────────────
// El sensor se comunica por UART a 9600 baud (TTL, 3.3V-5V tolerante en TX
// pero conviene usar un divisor de tensión en el RX del sensor si lo alimentás a 5V).
// Pines libres en el Heltec WiFi LoRa 32 V2 (no chocan con OLED ni SX1276):

extern int co2;
extern float temp;

// Trama de comando oficial de Semtech / Winsen para solicitar concentración de CO2 (0x86)
const uint8_t cmdReadCO2[9] = {0xFF, 0x01, 0x86, 0x00, 0x00, 0x00, 0x00, 0x00, 0x79};

bool leerMHZ19_UART0(int &ppm);

#endif