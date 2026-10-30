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
/* Prepares the payload of the frame */
static void prepareTxFrame(uint8_t port)
{
    /*appData size is LORAWAN_APP_DATA_MAX_SIZE which is defined in "commissioning.h".
     *appDataSize max value is LORAWAN_APP_DATA_MAX_SIZE.
     *if enabled AT, don't modify LORAWAN_APP_DATA_MAX_SIZE, it may cause system hanging or failure.
     *if disabled AT, LORAWAN_APP_DATA_MAX_SIZE can be modified, the max value is reference to lorawan region and SF.
     *for example, if use REGION_CN470,
     *the max value for different DR can be found in MaxPayloadOfDatarateCN470 refer to DataratesCN470 and BandwidthsCN470 in "RegionCN470.h".
     */
    // int co2 = myMHZ19.getCO2();              // ppm, 0-5000 típico
    // float temp = myMHZ19.getTemperature();   // temperatura interna del sensor (menos precisa que un DHT)

    co2 = 0;
    if (!leerMHZ19_UART0(co2))
    {

        // Si la lectura falla, enviamos 0xFFFF en ambos campos como señal de error
        co2 = -1;
        // Serial.println("[WARN] Enviando valores de error por fallo del sensor.");
    }

    // CO2: entero directo, 0-5000 entra en 16 bits sin problema.
    // Temperatura: la mandamos x10 igual que antes para no usar floats en el payload.
    uint16_t co2Val = (uint16_t)co2;
    int16_t tempInt = (int16_t)(temp * 10);
    float vPin37 = 0.0f;
    float vbatCalculado = medirVoltajeBateria(vPin37);
    uint16_t vbatEncoded = (uint16_t)(vbatCalculado * 1000.0f); // en V

    // Serial.printf("CO2: %d ppm  Temp: %.1f°C\n", co2, temp);

    appDataSize = 6;
    appData[0] = (co2Val >> 8) & 0xFF;      // byte alto CO2
    appData[1] = co2Val & 0xFF;             // byte bajo CO2
    appData[2] = (tempInt >> 8) & 0xFF;     // byte alto temperatura
    appData[3] = tempInt & 0xFF;            // byte bajo temperatura
    appData[4] = (vbatEncoded >> 8) & 0xFF; // byte alto de batería (V)
    appData[5] = vbatEncoded & 0xFF;        // byte bajo de batería (V)
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
        sendStartMs = millis(); // armar
        LoRaWAN.displaySending();
        prepareTxFrame(appPort);
        LoRaWAN.send();
        sendStartMs = 0; // desarmar
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
