#ifndef _MODEM_h
#define _MODEM_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
#include "WProgram.h"
#endif // defined(ARDUINO) && ARDUINO >= 100

#define TINY_GSM_MODEM_SIM7000SSL // Define modem model

#include <TinyGsmClient.h>

class SIM7000G {
private:
	TinyGsmSim7000SSL modem;

public:
	SIM7000G(Stream& stream) : modem(stream) {}

	// Restart modem.
	bool restart();

	// Maintain modem connection.
	void maintain();


	// Get modem manufacturer.
	String getHardwareManufacturer();

	// Get modem model.
	String getHardwareModel();

	// Get modem firmware version.
	String getFirmwareVersion();


	// Set preferred modem network mode. Returns true if successful.
	// 2 = Automatic
	// 13 = GSM only
	// 38 = LTE only
	// 51 = GSM and LTE only
	bool setNetworkMode(uint8_t mode, bool retry);

	// Set preferred modem radio mode. Returns true if successful.
	// 1 = CAT-M
	// 2 = NB-Iot
	// 3 = CAT-M and NB-IoT
	bool setRadioMode(uint8_t mode, bool retry);

	// Set PDP (packet data protocol) context parameters. Returns true if successful.
	bool setPDPParams();

	// Set the UE (user equipment) RF curcuit RX/TX power mode (circuit functionality).
	// 0 = Minimum functionality
	// 1 = Full functionality (Default)
	// 4 = Disable both RX and TX RF circuits
	// 5 = Factory test mode
	// 6 = Reset
	// 7 = Offline Mode
	bool setUEFunctionality(uint8_t mode);


	// Unlock SIM PIN. Returns true if successful or if SIM is already unlocked.
	bool unlockSIM();

	// Wait for a network connection. Returns true if connected within the timeout.
	bool connectToNetwork(uint32_t timeoutMs);


	// Get modem IMEI.
	String getIMEI();

	// Get SIM IMSI.
	String getIMSI();

	// Get SIM phone number.
	String getPhoneNumber();


	// Get network operator.
	String getNetworkOperator();

	// Get network signal quality.
	String getNetworkSignalQuality();

	// Get network connection type.
	String getNetworkConnectionType();

	// Get network local IP address.
	String getLocalIpAddress();


	// Send an SMS.
	bool sendSMS(String recipient, String message);
};

#endif // _MODEM_h
