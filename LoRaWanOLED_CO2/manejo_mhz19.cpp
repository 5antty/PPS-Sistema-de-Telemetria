#include "manejo_mhz19.h"

// Función para calcular la suma de comprobación (Checksum) del protocolo MH-Z19C
uint8_t calcularChecksum(uint8_t *packet)
{
    uint8_t checksum = 0;
    for (int i = 1; i < 8; i++)
    {
        checksum += packet[i];
    }
    checksum = 0xFF - checksum + 0x01;
    return checksum;
}

// Función para solicitar y leer el valor de CO2 desde UART0 (Serial)
bool leerMHZ19_UART0(int &ppm)
{
    // Limpiar el búfer de entrada de UART0 antes de enviar el comando
    while (Serial.available() > 0)
    {
        Serial.read();
    }

    // Enviar comando de lectura de 9 bytes
    Serial.write(cmdReadCO2, 9);
    Serial.flush();

    // Esperar a recibir la respuesta de 9 bytes (tiempo de espera límite: 100 ms)
    unsigned long inicioEspera = millis();
    while (Serial.available() < 9)
    {
        if (millis() - inicioEspera > 100)
        {
            return false; // Tiempo de espera agotado (Timeout)
        }
    }

    // Leer la respuesta de 9 bytes
    uint8_t respuesta[9];
    Serial.readBytes(respuesta, 9);

    // Validar cabecera, comando y checksum
    if (respuesta[0] == 0xFF && respuesta[1] == 0x86)
    {
        uint8_t csCalculado = calcularChecksum(respuesta);
        if (respuesta[8] == csCalculado)
        {
            // Cálculo oficial: CO2 = HIGH * 256 + LOW
            ppm = ((int)respuesta[2] * 256) + (int)respuesta[3];
            return true;
        }
    }

    return false;
}