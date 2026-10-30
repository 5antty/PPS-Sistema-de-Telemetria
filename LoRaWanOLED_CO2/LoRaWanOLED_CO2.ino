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

#include "manejo_lorawan.h"
#include "medir_vbat.h"
#include "manejo_mhz19.h"

/* OTAA para*/

uint8_t appEui[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};                                                 // LSB
uint8_t devEui[] = {0x70, 0xB3, 0xD5, 0x7E, 0xD0, 0x07, 0x8B, 0xC1};                                                 // MSB
uint8_t appKey[] = {0xFA, 0x6F, 0x18, 0xD7, 0x0A, 0xEC, 0x30, 0x72, 0xDC, 0x1B, 0x07, 0x26, 0xA9, 0xC2, 0x70, 0x51}; // MSB

/* ABP para*/
uint8_t nwkSKey[] = {0x15, 0xb1, 0xd0, 0xef, 0xa4, 0x63, 0xdf, 0xbe, 0x3d, 0x11, 0x18, 0x1e, 0x1e, 0xc7, 0xda, 0x85};
uint8_t appSKey[] = {0xd7, 0x2c, 0x78, 0x75, 0x8c, 0xdc, 0xca, 0xbf, 0x55, 0xee, 0x4a, 0x77, 0x8d, 0x16, 0xef, 0x67};
uint32_t devAddr = (uint32_t)0x007e6ae1;

/*LoraWan channelsmask*/
// Tiene que ser así (sub-banda 2, canales 8-15):
uint16_t userChannelsMask[6] = {0xFF00, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000};

/*LoraWan region, select in arduino IDE tools*/
LoRaMacRegion_t loraWanRegion = ACTIVE_REGION;

/*LoraWan Class, Class A and Class C are supported*/
DeviceClass_t loraWanClass = CLASS_A;

/*the application data transmission duty cycle.  value in [ms].*/
uint32_t appTxDutyCycle = 15000;

/*OTAA or ABP*/
bool overTheAirActivation = true;

/*ADR enable*/
bool loraWanAdr = true;

/* Indicates if the node is sending confirmed or unconfirmed messages */
bool isTxConfirmed = false;

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

int co2 = 0;
float temp = 20.0;

RTC_DATA_ATTR bool firstrun = true;

void setup()
{
  // Serial.begin(115200);

  // mhzSerial.begin(9600, SERIAL_8N1, MHZ19_RX_PIN, MHZ19_TX_PIN);
  // myMHZ19.begin(mhzSerial);
  // myMHZ19.autoCalibration(false); // desactiva ABC; activalo (true) si el sensor va a estar en exterior con ventilación regular

  // Configurar UART0 (Serial) exactamente a 9600 baudios (requerido por el MH-Z19C)
  Serial.begin(9600);

  Mcu.begin(HELTEC_BOARD, SLOW_CLK_TPYE);
  if (firstrun)
  {
    LoRaWAN.displayMcuInit();
    firstrun = false;
  }
  delay(2000); // Pausa breve de inicio
}

void loop()
{
  actualizarEstadoLoRaWAN();
}
