#ifndef _MODEM_h
#define _MODEM_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
#include "WProgram.h"
#endif // defined(ARDUINO) && ARDUINO >= 100

// Get modem manufacturer.
String modem_GetHardwareManufacturer();

// Get modem model.
String modem_GetHardwareModel();

// Get modem firmware version.
String modem_GetFirmwareVersion();


// Set preferred modem network mode. Returns true if successful.
// 2 = Automatic
// 13 = GSM only
// 38 = LTE only
// 51 = GSM and LTE only
bool modem_SetNetworkMode(uint8_t mode, bool retry);

// Set preferred modem radio mode. Returns true if successful.
// 1 = CAT-M
// 2 = NB-Iot
// 3 = CAT-M and NB-IoT
bool modem_SetRadioMode(uint8_t mode, bool retry);

// Set PDP (packet data protocol) context parameters. Returns true if successful.
bool modem_SetPdpParams();

// Set the UE (user equipment) RF curcuit RX/TX power mode (circuit functionality).
// 0 = Minimum functionality
// 1 = Full functionality (Default)
// 4 = Disable both RX and TX RF circuits
// 5 = Factory test mode
// 6 = Reset
// 7 = Offline Mode
bool modem_SetUeFunctionality(uint8_t mode);


// Unlock SIM PIN. Returns true if successful or if SIM is already unlocked.
bool modem_UnlockSimPin();

// Wait for a network connection. Returns true if connected within the timeout.
bool modem_ConnectToNetwork(uint32_t timeoutMs);


// Get modem IMEI.
String modem_GetImei();

// Get SIM IMSI.
String modem_GetImsi();

// Get SIM phone number.
String modem_GetPhoneNumber();


// Get network operator.
String modem_GetNetworkOperator();

// Get network signal quality.
String modem_GetNetworkSignalQuality();

// Get network connection type.
String modem_GetNetworkConnectionType();

// Get network local IP address.
String modem_GetLocalIpAddress();

#endif // _MODEM_h
