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
 * Decodificador para TTN / ChirpStack:
 *   function decodeUplink(input) {
 *     var temp = ((input.bytes[0] << 8) | input.bytes[1]) / 10.0;
 *     var hum  = ((input.bytes[2] << 8) | input.bytes[3]) / 10.0;
 *     return { data: { temperature: temp, humidity: hum } };
 *   }
 */

#include <Arduino.h>
#include "LoRaWan_APP.h"
#include "DHT.h"

// ─── Configuración del DHT11 ──────────────────────────────────────────────────
#define DHTPIN  13        // GPIO conectado al pin DATA del DHT11
                          // Cambiarlo según tu cableado (pines libres en V2:
                          // 13, 17, 22, 23, etc.)
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);

// Variables globales para los datos del sensor
float temperature = 0.0;
float humidity    = 0.0;

// ─── Credenciales OTAA ────────────────────────────────────────────────────────
// ⚠️  Reemplazá estos valores con los de tu red (TTN, ChirpStack, etc.)

// AppEUI / JoinEUI: identificador de la aplicación
uint8_t appEui[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
// DevEUI: identificador único del dispositivo (LSB first en algunos servidores)
//70B3D57ED00778A6
uint8_t devEui[] = {0x70, 0xB3, 0xD5, 0x7E, 0xD0, 0x07, 0x8B, 0xC0};

// AppKey: clave de cifrado de 16 bytes
//7A 84 28 EC 40 C8 58 24 0E 62 AA CB F7 83 DD 85
uint8_t appKey[] = {0xD5, 0x6B, 0x0D, 0xC1, 0x5E, 0x6A, 0x29, 0xA9, 0x6B, 0xA8, 0x98, 0x2D, 0xB1, 0x32, 0x65, 0x33};

/* Parámetros ABP (no se usan en OTAA, pero la librería los requiere declarados) */
uint8_t nwkSKey[] = {0x35, 0x5F, 0xC1, 0x1E, 0x19, 0x11, 0x81, 0x1D, 0x11, 0xF7, 0xE4, 0x7D, 0x21, 0x46, 0x2B, 0xE6};
uint8_t appSKey[] = {0x72, 0xB2, 0x15, 0x11, 0x29, 0xEE, 0xA9, 0x6B, 0xEE, 0x2E, 0xA7, 0x31, 0x47, 0xC8, 0x04, 0xFF};
uint32_t devAddr = (uint32_t)0x260CCF19;

// ─── Configuración LoRaWAN ────────────────────────────────────────────────────
//   Configuración para la Sub-banda 2 de TTN (Canales 8-15)
uint16_t userChannelsMask[6] = { 0xFF00, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000 };

LoRaMacRegion_t loraWanRegion = ACTIVE_REGION; // Definir en Tools → LoRaWAN Region

DeviceClass_t loraWanClass = CLASS_A;

uint32_t appTxDutyCycle = 15000; // Intervalo entre envíos en ms (mínimo recomendado: 15000)

bool overTheAirActivation = true; // ← OTAA desactivado

bool loraWanAdr = true;           // Adaptive Data Rate habilitado

bool isTxConfirmed = false;        // Mensajes NO confirmados (el servidor NO responde ACK)

uint8_t appPort = 2;

uint8_t confirmedNbTrials = 4;    // Reintentos ante falta de ACK

// ─── Lectura del sensor ───────────────────────────────────────────────────────
/**
 * Lee el DHT11 y actualiza las variables globales.
 * Retorna true si la lectura fue válida.
 */
bool readDHT11() {
  float h = dht.readHumidity();
  float t = dht.readTemperature(); // Celsius por defecto

  if (isnan(h) || isnan(t)) {
    Serial.println("[DHT11] Error: lectura inválida");
    return false;
  }

  humidity    = h;
  temperature = t;

  Serial.printf("[DHT11] Temperatura: %.1f °C  |  Humedad: %.1f %%\n", temperature, humidity);
  return true;
}

// ─── Preparación del payload ──────────────────────────────────────────────────
/**
 * Codifica temperatura y humedad en 4 bytes (big-endian, x10).
 *
 * Formato:
 *   [0..1] temperatura * 10  →  25.3°C = 253 = 0x00FD
 *   [2..3] humedad    * 10  →  61.0%  = 610 = 0x0262
 *
 * (El DHT11 solo entrega enteros, pero se multiplica x10 para
 *  mantener compatibilidad con el decodificador si luego se usa
 *  un sensor con decimales como el DHT22.)
 */
static void prepareTxFrame(uint8_t port) {
  if (!readDHT11()) {
    // Si la lectura falla, enviamos 0xFFFF en ambos campos como señal de error
    temperature = -1.0;
    humidity    = -1.0;
    Serial.println("[WARN] Enviando valores de error por fallo del sensor.");
  }

  uint16_t tempEncoded = (uint16_t)(temperature * 10);
  uint16_t humEncoded  = (uint16_t)(humidity    * 10);

  appDataSize = 4;
  appData[0] = (tempEncoded >> 8) & 0xFF; // byte alto de temperatura
  appData[1] =  tempEncoded       & 0xFF; // byte bajo de temperatura
  appData[2] = (humEncoded  >> 8) & 0xFF; // byte alto de humedad
  appData[3] =  humEncoded        & 0xFF; // byte bajo de humedad

  Serial.printf("[TX] Payload: %02X %02X %02X %02X\n",
                appData[0], appData[1], appData[2], appData[3]);
}

// ─── Setup y loop ─────────────────────────────────────────────────────────────
RTC_DATA_ATTR bool firstrun = true;

void setup() {
  Serial.begin(115200);

  dht.begin();
  Serial.println("[DHT11] Sensor iniciado.");

  Mcu.begin(HELTEC_BOARD, SLOW_CLK_TPYE);

#ifdef WIFI_LORA_32_V4
  pinMode(Vext, OUTPUT);
  digitalWrite(Vext, LOW);
#endif

  if (firstrun) {
    LoRaWAN.displayMcuInit();
    firstrun = false;
  }
}

void loop() {
  switch (deviceState) {
    case DEVICE_STATE_INIT: {
      LoRaWAN.init(loraWanClass, loraWanRegion);
      LoRaWAN.setDefaultDR(5); // DR3 = SF9 en la mayoría de regiones
      break;
    }

    case DEVICE_STATE_JOIN: {
      LoRaWAN.displayJoining();
      LoRaWAN.join(); // Intenta el handshake OTAA (Join Request / Join Accept)
      break;
    }

    case DEVICE_STATE_SEND: {
      LoRaWAN.displaySending();
      prepareTxFrame(appPort);
      LoRaWAN.send();
      deviceState = DEVICE_STATE_CYCLE;
      break;
    }

    case DEVICE_STATE_CYCLE: {
      txDutyCycleTime = appTxDutyCycle + randr(-APP_TX_DUTYCYCLE_RND, APP_TX_DUTYCYCLE_RND);
      LoRaWAN.cycle(txDutyCycleTime);
      deviceState = DEVICE_STATE_SLEEP;
      break;
    }

    case DEVICE_STATE_SLEEP: {
      LoRaWAN.displayAck();
      LoRaWAN.sleep(loraWanClass);
      break;
    }

    default: {
      deviceState = DEVICE_STATE_INIT;
      break;
    }
  }
}
