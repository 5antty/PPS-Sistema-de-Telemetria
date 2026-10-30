#ifndef MEDIR_VBAT_H
#define MEDIR_VBAT_H

#include <Arduino.h>

// Medir vbat

#define ADC_CTRL_PIN 21 // GPIO21 = Control de MOSFET Q3 (P-FET: LOW = Conecta batería al divisor)
#define VBAT_ADC_PIN 37 // GPIO37 = ADC1_CH1 (Punto medio del divisor R10/R12 en V2.1)

// Factor de división calibrado mediante multímetro
#define DIVIDER_RATIO 3.175f

float medirVoltajeBateria(float &rawVoltsPin);

#endif