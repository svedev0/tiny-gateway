#ifndef _GLOBALS_h
#define _GLOBALS_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
#include "WProgram.h"
#endif // defined(ARDUINO) && ARDUINO >= 100

// GPIO map
#define LED_PIN           12
#define MODEM_POWER_PIN   4
#define MODEM_UART_RX_PIN 26
#define MODEM_UART_TX_PIN 27
#define SD_MISO_PIN       2
#define SD_MOSI_PIN       15
#define SD_SCK_PIN        14
#define SD_CS_PIN         13
#define BATTERY_ADC_PIN   35
#define SOLAR_ADC_PIN     36

// USB UART
#define UsbSerial          Serial0
#define USB_UART_BAUD      115200

// SIM7000G UART
#define ModemSerial        Serial1
#define MODEM_UART_BAUD    115200
#define MODEM_UART_CFG     SERIAL_8N1
#define TINY_GSM_RX_BUFFER 1024  // 1KB

// SIM APN
#define APN (char*)"example-apn.com"

// WiFi access point
#define AP_SSID     (char*)"TinyGateway"
#define AP_PASSWORD (char*)"hl54b6xwp2n6rxey"

// HTTP server
#define HTTP_PORT       80
#define HTTP_AUTH_TOKEN (char*)"mi5aj9rwkdd4x68p"

// Logging
#define VERBOSE_LOGGING false

// If you do not have complete understanding of the code in this project, do not
// change this value. Setting this to false will skip important configuration steps.
// This should only be used by advanced users for testing purposes.
#define I_KNOW_WHAT_I_AM_DOING true

#endif // _GLOBALS_h
