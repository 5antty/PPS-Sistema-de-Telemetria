## Resumen

Este informe documenta los problemas encontrados y las soluciones aplicadas durante la configuración inicial de un nodo Heltec WiFi LoRa 32 V2 para transmisión LoRaWAN.

#### Problema 1: Elección del entorno de desarrollo

En un principio se planeaba trabajar con **PlatformIO + VSCode**, entorno preferido por comodidad y flujo de trabajo. Sin embargo, dado que la biblioteca de Heltec está desarrollada y mantenida pensando en **Arduino IDE**, se optó finalmente por utilizar este último para evitar incompatibilidades adicionales.

#### Problema 2: Errores de compilación en los ejemplos de la biblioteca Heltec

Al intentar compilar los sketches de ejemplo provistos por la biblioteca oficial de Heltec, el compilador arrojaba errores relacionados con variables no definidas y redefiniciones de símbolos. Tras investigar, se determinó que la causa raíz era la versión del paquete de boards de Heltec para Arduino: las actualizaciones más recientes habían discontinuado el soporte para la placa V2, generando estos conflictos de compilación.

**Solución:** se identificó que la última versión del paquete de boards compatible con la V2 era la **3.0.3**. Fijando esa versión específica en el entorno de desarrollo, los ejemplos compilaron sin inconvenientes. Con respecto a la biblioteca de LoRaWAN en sí, no fue necesario fijar una versión particular; la última estable funcionó correctamente.

#### Problema 3: Intento fallido con la biblioteca LMIC

Antes de lograr una comunicación LoRaWAN exitosa, y sin haber detectado aún que el problema real eran ciertos parámetros de configuración en Arduino IDE, se intentó como alternativa utilizar la **biblioteca LMIC** para establecer la comunicación entre el nodo y el gateway. Por motivos que no se llegaron a determinar con claridad, esta vía no funcionó, por lo que se abandonó y se volvió a la biblioteca oficial de Heltec.

#### Problema 4: Los mensajes no llegaban al gateway

Con la biblioteca de Heltec ya compilando correctamente, el nodo transmitía pero los mensajes LoRaWAN no eran recibidos por el gateway. Esto se debía a que no se habían configurado correctamente los parámetros de Arduino IDE referidos a LoRaWAN.

**Solución:** se ajustaron los siguientes parámetros de configuración en el código:

- Activación del modo de activación **OTAA** (Over-The-Air Activation).
- Configuración para utilizar el **canal 2 de la sub-banda** correspondiente.23
- Configuracion de frecuencia/region LoRaWAN, pues por defecto esta en EU_433 y hay que ponerla en AU_915.

Con estos cambios, los mensajes comenzaron a llegar correctamente al gateway.

#### Conclusión

El proceso de puesta en marcha requirió varios ajustes: adaptar el entorno de desarrollo a las limitaciones de la biblioteca de Heltec (Arduino IDE en lugar de PlatformIO), resolver incompatibilidades de versión del paquete de boards (fijando la 3.0.3), descartar una vía alternativa con LMIC que no dio resultado, y finalmente corregir los parámetros de OTAA y sub-banda/canal para lograr la comunicación con el gateway.