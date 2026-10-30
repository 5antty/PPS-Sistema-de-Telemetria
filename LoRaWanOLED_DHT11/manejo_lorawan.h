#ifndef MANEJO_LORAWAN_H
#define MANEJO_LORAWAN_H

#include <Arduino.h>
#include "LoRaWan_APP.h"

#include "medir_vbat.h"
#include "manejo_dht.h"
#include "manejo_watchdog.h"

static void prepareTxFrame(uint8_t port);
void actualizarEstadoLoRaWAN();

#endif