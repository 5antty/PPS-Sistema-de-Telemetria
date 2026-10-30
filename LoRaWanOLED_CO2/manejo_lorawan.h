#ifndef MANEJO_LORAWAN_H
#define MANEJO_LORAWAN_H

#include <Arduino.h>
#include "LoRaWan_APP.h"

#include "medir_vbat.h"
#include "manejo_mhz19.h"

static void prepareTxFrame(uint8_t port);
void actualizarEstadoLoRaWAN();

#endif