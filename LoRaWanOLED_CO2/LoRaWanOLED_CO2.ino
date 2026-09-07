/* Heltec Automation LoRaWAN communication example
 *
 * Function:
 * 1. Upload node data to the server using the standard LoRaWAN protocol.
 * 2. The network access status of LoRaWAN is displayed on the screen.
 * 
 * Description:
 * 1. Communicate using LoRaWAN protocol.
 * 
 * HelTec AutoMation, Chengdu, China
 * 成都惠利特自动化科技有限公司
 * www.heltec.org
 *
 * this project also realess in GitHub:
 * https://github.com/Heltec-Aaron-Lee/WiFi_Kit_series
 * */

// the Arduino build environment automatically includes Arduino.h for .ino
// sketches, so we don’t need to #include it and avoid the “cannot open source
// file" error.
// add the core header explicitly so editors/linters with a broken includePath
// can still resolve the dependency
#include <Arduino.h>
#include "LoRaWan_APP.h"
#include <MHZ19.h>          // Librería "MHZ19" de Jonathan Dempsey (Library Manager)

/* OTAA para*/
uint8_t devEui[] = {0x70, 0xB3, 0xD5, 0x7E, 0xD0, 0x07, 0x82, 0x1B}; // MSB
uint8_t appEui[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; // LSB
uint8_t appKey[] = {0xD9, 0xAD, 0x0F, 0x4C, 0x59, 0x59, 0x29, 0xDB, 0x37, 0x14, 0xE8, 0x90, 0xF7, 0x1D, 0xA8, 0x9A}; // MSB

/* ABP para*/
uint8_t nwkSKey[] = { 0x15, 0xb1, 0xd0, 0xef, 0xa4, 0x63, 0xdf, 0xbe, 0x3d, 0x11, 0x18, 0x1e, 0x1e, 0xc7, 0xda,0x85 };
uint8_t appSKey[] = { 0xd7, 0x2c, 0x78, 0x75, 0x8c, 0xdc, 0xca, 0xbf, 0x55, 0xee, 0x4a, 0x77, 0x8d, 0x16, 0xef,0x67 };
uint32_t devAddr =  ( uint32_t )0x007e6ae1;

/*LoraWan channelsmask*/
// Tiene que ser así (sub-banda 2, canales 8-15):
uint16_t userChannelsMask[6] = { 0xFF00, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000 };

/*LoraWan region, select in arduino IDE tools*/
LoRaMacRegion_t loraWanRegion = ACTIVE_REGION;

/*LoraWan Class, Class A and Class C are supported*/
DeviceClass_t  loraWanClass = CLASS_A;

/*the application data transmission duty cycle.  value in [ms].*/
uint32_t appTxDutyCycle = 15000;

/*OTAA or ABP*/
bool overTheAirActivation = true;

/*ADR enable*/
bool loraWanAdr = true;


/* Indicates if the node is sending confirmed or unconfirmed messages */
bool isTxConfirmed = true;

/* Application port */
uint8_t appPort = 2;
/*!
* Number of trials to transmit the frame, if the LoRaMAC layer did not
* receive an acknowledgment. The MAC performs a datarate adaptation,
* according to the LoRaWAN Specification V1.0.2, chapter 18.4, according
* to the following table:
*
* Transmission nb | Data Rate
* ----------------|-----------
* 1 (first)       | DR
* 2               | DR
* 3               | max(DR-1,0)
* 4               | max(DR-1,0)
* 5               | max(DR-2,0)
* 6               | max(DR-2,0)
* 7               | max(DR-3,0)
* 8               | max(DR-3,0)
*
* Note, that if NbTrials is set to 1 or 2, the MAC will not decrease
* the datarate, in case the LoRaMAC layer did not receive an acknowledgment
*/
uint8_t confirmedNbTrials = 4;

// ─── MH-Z19B (CO2) ─────────────────────────────────────────────────────────
// El sensor se comunica por UART a 9600 baud (TTL, 3.3V-5V tolerante en TX
// pero conviene usar un divisor de tensión en el RX del sensor si lo alimentás a 5V).
// Pines libres en el Heltec WiFi LoRa 32 V2 (no chocan con OLED ni SX1276):
#define MHZ19_RX_PIN 23   // ESP32 RX  <- MHZ19 TX (pin 5 del sensor)
#define MHZ19_TX_PIN 25   // ESP32 TX  -> MHZ19 RX (pin 6 del sensor)

HardwareSerial mhzSerial(1);   // usamos UART1 del ESP32
MHZ19 myMHZ19;

/* Prepares the payload of the frame */
static void prepareTxFrame( uint8_t port )
{
	/*appData size is LORAWAN_APP_DATA_MAX_SIZE which is defined in "commissioning.h".
	*appDataSize max value is LORAWAN_APP_DATA_MAX_SIZE.
	*if enabled AT, don't modify LORAWAN_APP_DATA_MAX_SIZE, it may cause system hanging or failure.
	*if disabled AT, LORAWAN_APP_DATA_MAX_SIZE can be modified, the max value is reference to lorawan region and SF.
	*for example, if use REGION_CN470, 
	*the max value for different DR can be found in MaxPayloadOfDatarateCN470 refer to DataratesCN470 and BandwidthsCN470 in "RegionCN470.h".
	*/
    int co2 = myMHZ19.getCO2();              // ppm, 0-5000 típico
    float temp = myMHZ19.getTemperature();   // temperatura interna del sensor (menos precisa que un DHT)

    if (co2 <= 0) {
        Serial.println("Error leyendo MH-Z19B");
        appDataSize = 0;
        return;
    }

    // CO2: entero directo, 0-5000 entra en 16 bits sin problema.
    // Temperatura: la mandamos x10 igual que antes para no usar floats en el payload.
    uint16_t co2Val  = (uint16_t)co2;
    int16_t  tempInt = (int16_t)(temp * 10);

    Serial.printf("CO2: %d ppm  Temp: %.1f°C\n", co2, temp);

    appDataSize = 4;
    appData[0] = (co2Val   >> 8) & 0xFF;  // byte alto CO2
    appData[1] =  co2Val         & 0xFF;  // byte bajo CO2
    appData[2] = (tempInt  >> 8) & 0xFF;  // byte alto temperatura
    appData[3] =  tempInt        & 0xFF;  // byte bajo temperatura
}

RTC_DATA_ATTR bool firstrun = true;

void setup() {
  Serial.begin(115200);

  mhzSerial.begin(9600, SERIAL_8N1, MHZ19_RX_PIN, MHZ19_TX_PIN);
  myMHZ19.begin(mhzSerial);
  myMHZ19.autoCalibration(false); // desactiva ABC; activalo (true) si el sensor va a estar en exterior con ventilación regular

  Mcu.begin(HELTEC_BOARD,SLOW_CLK_TPYE);
#ifdef WIFI_LORA_32_V4
  pinMode(Vext, OUTPUT);
  digitalWrite(Vext, LOW);
#endif
  if(firstrun)
  {
    LoRaWAN.displayMcuInit();
    firstrun = false;
  }

}

void loop()
{
	switch( deviceState )
	{
		case DEVICE_STATE_INIT:
		{
#if(LORAWAN_DEVEUI_AUTO)
			LoRaWAN.generateDeveuiByChipID();
#endif
			LoRaWAN.init(loraWanClass,loraWanRegion);
			//both set join DR and DR when ADR off 
			LoRaWAN.setDefaultDR(3);
			break;
		}
		case DEVICE_STATE_JOIN:
		{
			LoRaWAN.displayJoining();
			LoRaWAN.join();
			break;
		}
		case DEVICE_STATE_SEND:
		{
			LoRaWAN.displaySending();
			prepareTxFrame( appPort );
			LoRaWAN.send();
			deviceState = DEVICE_STATE_CYCLE;
			break;
		}
		case DEVICE_STATE_CYCLE:
		{
			// Schedule next packet transmission
			txDutyCycleTime = appTxDutyCycle + randr( -APP_TX_DUTYCYCLE_RND, APP_TX_DUTYCYCLE_RND );
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
