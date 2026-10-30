#include "medir_vbat.h"

float medirVoltajeBateria(float &rawVoltsPin)
{
    // 1. Activar MOSFET Q3 poniendo GPIO21 en LOW para conectar el divisor al ADC
    pinMode(ADC_CTRL_PIN, OUTPUT);
    digitalWrite(ADC_CTRL_PIN, LOW);
    delay(10); // Estabilización del nodo analógico

    // 2. Configurar atenuación de 11dB en GPIO37 para medir rango amplio
    analogSetPinAttenuation(VBAT_ADC_PIN, ADC_11db);
    uint32_t sumamV = 0;
    const int MUESTRAS = 25;

    // Promediar 25 muestras para reducir ruido
    for (int i = 0; i < MUESTRAS; i++)
    {
        sumamV += analogReadMilliVolts(VBAT_ADC_PIN);
        delay(2);
    }

    // 3. Desconectar MOSFET Q3 poniendo GPIO21 en HIGH para preservar batería
    // digitalWrite(ADC_CTRL_PIN, HIGH);

    float mV_promedio = (float)sumamV / (float)MUESTRAS;
    rawVoltsPin = (mV_promedio / 1000.0f); // Voltaje directo medido en el pin GPIO37 (en Voltios)

    // Voltaje calculado a la entrada del divisor (VBAT)
    return rawVoltsPin * DIVIDER_RATIO;
}