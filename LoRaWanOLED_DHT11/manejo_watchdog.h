#ifndef MANEJO_WATCHDOG_H
#define MANEJO_WATCHDOG_H

#include "esp_system.h"
#include <Arduino.h>

extern volatile uint32_t sendStartMs;    // 0 = guardián desarmado
extern RTC_DATA_ATTR uint32_t wdtResets; // cuántas veces actuó (sobrevive al reinicio)
const uint32_t SEND_TIMEOUT_MS = 30000;

extern RTC_DATA_ATTR bool firstrun; // variable existente, debe declararse antes de la tarea

void sendWatchdogTask(void *pv);

#endif