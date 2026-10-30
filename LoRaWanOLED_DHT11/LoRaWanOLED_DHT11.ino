/* Heltec WiFi LoRa 32 V2 — LoRaWAN OTAA + DHT11
 *
 * Función:
 *   - Autenticación OTAA (Over The Air Activation)
 *   - Lectura de temperatura y humedad con DHT11
 *   - Envío del payload por LoRaWAN cada 15 segundos
 *   - Estado mostrado en pantalla OLED
 *
 * Librería requerida: "DHT sensor library" de Adafruit
 *   (instalar desde el gestor de librerías del IDE de Arduino,
 *    incluye automáticamente "Adafruit Unified Sensor")
 *
 * Payload (4 bytes, big-endian):
 *   Bytes 0-1: temperatura * 10  (ej. 25.0°C → 0x00FA)
 *   Bytes 2-3: humedad    * 10  (ej. 60.0%  → 0x0258)
 *
 * Decodificador para TTN:
 *   function decodeUplink(input) {
 *     var temp = ((input.bytes[0] << 8) | input.bytes[1]) / 10.0;
 *     var hum  = ((input.bytes[2] << 8) | input.bytes[3]) / 10.0;
 *     return { data: { temperature: temp, humidity: hum } };
 *   }
 */

#include "manejo_lorawan.h"
#include "medir_vbat.h"
#include "manejo_dht.h"

// Variables globales para los datos del sensor
float temperature = 0.0;
float humidity = 0.0;

// ─── Credenciales OTAA ────────────────────────────────────────────────────────
// Reemplazo estos valores con los de TTN

// AppEUI / JoinEUI: identificador de la aplicación
uint8_t appEui[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
// DevEUI: identificador único del dispositivo (LSB first en algunos servidores)
uint8_t devEui[] = {0x70, 0xB3, 0xD5, 0x7E, 0xD0, 0x07, 0x8B, 0xC0};
// AppKey: clave de cifrado de 16 bytes
uint8_t appKey[] = {0xD5, 0x6B, 0x0D, 0xC1, 0x5E, 0x6A, 0x29, 0xA9, 0x6B, 0xA8, 0x98, 0x2D, 0xB1, 0x32, 0x65, 0x33};

/* Parámetros ABP (no se usan en OTAA, pero la librería los requiere declarados) */
uint8_t nwkSKey[] = {0x35, 0x5F, 0xC1, 0x1E, 0x19, 0x11, 0x81, 0x1D, 0x11, 0xF7, 0xE4, 0x7D, 0x21, 0x46, 0x2B, 0xE6};
uint8_t appSKey[] = {0x72, 0xB2, 0x15, 0x11, 0x29, 0xEE, 0xA9, 0x6B, 0xEE, 0x2E, 0xA7, 0x31, 0x47, 0xC8, 0x04, 0xFF};
uint32_t devAddr = (uint32_t)0x260CCF19;

// ─── Configuración LoRaWAN ────────────────────────────────────────────────────
//   Configuración para la Sub-banda 2 de TTN (Canales 8-15)
uint16_t userChannelsMask[6] = {0xFF00, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000};

LoRaMacRegion_t loraWanRegion = ACTIVE_REGION; // Definir en Tools → LoRaWAN Region

DeviceClass_t loraWanClass = CLASS_A;

uint32_t appTxDutyCycle = 15000; // Intervalo entre envíos en ms (mínimo recomendado: 15000)

bool overTheAirActivation = true; // ← OTAA activado

bool loraWanAdr = true; // Adaptive Data Rate habilitado

bool isTxConfirmed = false; // Mensajes NO confirmados (el servidor NO responde ACK)

uint8_t appPort = 2;

uint8_t confirmedNbTrials = 4; // Reintentos ante falta de ACK

// ─── Setup y loop ─────────────────────────────────────────────────────────────
RTC_DATA_ATTR bool firstrun = true;

void setup()
{
  Serial.begin(115200);

  analogReadResolution(12);

  initDHT();
  Serial.println("[DHT11] Sensor iniciado.");

  Mcu.begin(HELTEC_BOARD, SLOW_CLK_TPYE);

  if (firstrun)
  {
    LoRaWAN.displayMcuInit();
    firstrun = false;
  }
}

void loop()
{
  actualizarEstadoLoRaWAN();
}
