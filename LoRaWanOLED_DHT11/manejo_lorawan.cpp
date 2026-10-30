#include "manejo_lorawan.h"

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
static void prepareTxFrame(uint8_t port)
{
    if (!readDHT11())
    {
        // Si la lectura falla, enviamos 0xFFFF en ambos campos como señal de error
        temperature = -1.0;
        humidity = -1.0;
        Serial.println("[WARN] Enviando valores de error por fallo del sensor.");
    }
    float vPin37 = 0.0f;
    float vbatCalculado = medirVoltajeBateria(vPin37);

    uint16_t tempEncoded = (uint16_t)(temperature * 10);
    uint16_t humEncoded = (uint16_t)(humidity * 10);
    uint16_t vbatEncoded = (uint16_t)(vbatCalculado * 1000.0f); // en V

    appDataSize = 6;
    appData[0] = (tempEncoded >> 8) & 0xFF;
    appData[1] = tempEncoded & 0xFF;
    appData[2] = (humEncoded >> 8) & 0xFF;
    appData[3] = humEncoded & 0xFF;
    appData[4] = (vbatEncoded >> 8) & 0xFF; // byte alto de batería (V)
    appData[5] = vbatEncoded & 0xFF;        // byte bajo de batería (V)

    Serial.printf("[TX] Payload: %02X %02X %02X %02X %02X %02X (Vbat=%.3f V)\n",
                  appData[0], appData[1], appData[2], appData[3],
                  appData[4], appData[5], vbatCalculado);
}

void actualizarEstadoLoRaWAN()
{
    switch (deviceState)
    {
    case DEVICE_STATE_INIT:
    {
        LoRaWAN.init(loraWanClass, loraWanRegion);
        LoRaWAN.setDefaultDR(5); // DR3 = SF9 en la mayoría de regiones
        break;
    }

    case DEVICE_STATE_JOIN:
    {
        LoRaWAN.displayJoining();
        LoRaWAN.join(); // Intenta el handshake OTAA (Join Request / Join Accept)
        break;
    }

    case DEVICE_STATE_SEND:
    {
        LoRaWAN.displaySending();
        prepareTxFrame(appPort);
        LoRaWAN.send();
        deviceState = DEVICE_STATE_CYCLE;
        break;
    }

    case DEVICE_STATE_CYCLE:
    {
        txDutyCycleTime = appTxDutyCycle + randr(-APP_TX_DUTYCYCLE_RND, APP_TX_DUTYCYCLE_RND);
        LoRaWAN.cycle(txDutyCycleTime);
        deviceState = DEVICE_STATE_SLEEP;
        break;
    }

    case DEVICE_STATE_SLEEP:
    {
        LoRaWAN.displayAck();
        LoRaWAN.sleep(loraWanClass);
        break;
    }

    default:
    {
        deviceState = DEVICE_STATE_INIT;
        break;
    }
    }
}
