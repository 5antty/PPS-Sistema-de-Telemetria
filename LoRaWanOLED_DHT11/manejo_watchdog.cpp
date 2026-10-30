#include "manejo_watchdog.h"

void sendWatchdogTask(void *pv)
{
    for (;;)
    {
        uint32_t t0 = sendStartMs;
        if (t0 != 0 && (millis() - t0) > SEND_TIMEOUT_MS)
        {
            wdtResets++;
            firstrun = true; // para que displayMcuInit() reinicialice la OLED tras el reinicio
            esp_restart();
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}