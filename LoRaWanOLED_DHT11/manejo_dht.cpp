#include "manejo_dht.h"

DHT dht(DHTPIN, DHTTYPE);

void initDHT(){
  dht.begin();
}

// ─── Lectura del sensor ───────────────────────────────────────────────────────
/**
 * Lee el DHT11 y actualiza las variables globales.
 * Retorna true si la lectura fue válida.
 */
bool readDHT11()
{
    float h = dht.readHumidity();
    float t = dht.readTemperature(); // Celsius por defecto

    if (isnan(h) || isnan(t))
    {
        Serial.println("[DHT11] Error: lectura inválida");
        return false;
    }

    humidity = h;
    temperature = t;

    Serial.printf("[DHT11] Temperatura: %.1f °C  |  Humedad: %.1f %%\n", temperature, humidity);
    return true;
}
