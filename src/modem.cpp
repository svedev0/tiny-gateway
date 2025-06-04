#include "globals.h"
#include "modem.h"

bool SIM7000G::restart() {
	return modem.restart();
}

bool SIM7000G::ready() {
	return modem.testAT(); // AT
}

void SIM7000G::maintain() {
	modem.maintain();
}


String SIM7000G::getHardwareManufacturer() {
	return modem.getModemManufacturer(); // AT+CGMI
}

String SIM7000G::getHardwareModel() {
	return modem.getModemModel(); // AT+CGMM
}

String SIM7000G::getFirmwareVersion() {
	String fwVersion = modem.getModemRevision(); // AT+CGMR
	fwVersion.replace("Revision:", "");
	return fwVersion;
}

bool SIM7000G::setNetworkMode(const uint8_t mode, const bool retry) {
	if (!retry) {
		return modem.setNetworkMode(mode); // AT+CNMP
	}

	while (true) {
		if (modem.setNetworkMode(mode)) {
			break;
		}
		delay(200);
	}

	return true;
}

bool SIM7000G::setRadioMode(const uint8_t mode, const bool retry) {
	if (!retry) {
		return modem.setPreferredMode(mode); // AT+CMNB
	}

	while (true) {
		if (modem.setPreferredMode(mode)) {
			break;
		}
		delay(200);
	}

	return true;
}

bool SIM7000G::setPDPParams() {
	// Get defined PDP contexts
	ModemSerial.println("AT+CGACT?");
	delay(200);

	if (!ModemSerial.available()) {
		return false;
	}

	String input = ModemSerial.readString();
	if (input.indexOf("ERROR") >= 0) {
		return false;
	}

	if (input.indexOf("+CGACT:") < 0) {
		return false;
	}

	// Replace everything except the PDP context IDs
	input.replace("OK", "");
	input.replace("+CGACT: ", "");
	input.replace(",0", "");
	input.replace(",1", "");
	input.replace("\r", "");
	input.trim();
	input.replace("\n", "|"); // Pipe as separator
	input += "|"; // Ensure the last context is processed

	// Count the number of contexts
	int pipeCount = 0;
	for (int i = 0; i < input.length(); i++) {
		if (input[i] == '|') {
			pipeCount++;
		}
	}

	// If no contexts are defined, define PDP context 1
	if (pipeCount <= 0) {
		UsbSerial.println("AT+CGDCONT=1,\"IP\",\"" + String(APN) + "\"");
		delay(300);
		return true;
	}

	// Set value of each defined PDP context
	for (int i = 0; i < pipeCount; i++) {
		int index = input.indexOf('|');
		if (index < 0) {
			break;
		}

		String contextId = input.substring(0, index);
		ModemSerial.println("AT+CGDCONT=" + contextId + ",\"IP\",\"" + String(APN) + "\"");
		delay(300);

		input = input.substring(index + 1);
	}

	return true;
}

bool SIM7000G::setUEFunctionality(const uint8_t mode) {
	ModemSerial.println("AT+CFUN=" + String(mode));
	delay(1000);

	if (!ModemSerial.available()) {
		return false;
	}

	String input = ModemSerial.readString();
	if (input.indexOf("READY") < 0 && input.indexOf("OK") < 0) {
		return false;
	}

	return true;
}


bool SIM7000G::unlockSIM() {
	if (modem.getSimStatus() != 3) { // AT+CPIN
		return true; // SIM is unlocked
	}

#ifdef SIM_PIN
	return modem.simUnlock(SIM_PIN); // AT+CPIN
#else
	return false;
#endif
}

bool SIM7000G::connectToNetwork(const uint32_t timeoutMs) {
	if (!modem.waitForNetwork(timeoutMs)) { // AT+CGREG or AT+CEREG
		return false;
	}
	if (!modem.isNetworkConnected()) { // AT+CGREG or AT+CEREG
		return false;
	}
	return true;
}


String SIM7000G::getIMEI() {
	return modem.getIMEI(); // AT+CGSN
}

String SIM7000G::getIMSI() {
	return modem.getIMSI(); // AT+CIMI
}

String SIM7000G::getICCID() {
	return modem.getSimCCID(); // AT+CCID
}

String SIM7000G::getPhoneNumber() {
	ModemSerial.println("AT+CNUM");
	delay(1000);

	if (!ModemSerial.available()) {
		return String();
	}

	String result = ModemSerial.readString();
	result.replace("\r", "");
	result.replace("\n", "");
	result.replace("\"", "");
	result.replace("+CNUM: ,", "");

	int commaIdx = result.indexOf(',');
	if (commaIdx < 0) {
		return String();
	}

	String phoneNum = result.substring(0, commaIdx);
	if (phoneNum.length() <= 0) {
		return String();
	}

	return phoneNum;
}


String SIM7000G::getNetworkOperator() {
	return modem.getOperator(); // AT+COPS
}

String SIM7000G::getNetworkSignalQuality() {
	int16_t sq = modem.getSignalQuality(); // AT+CSQ
	return String("-" + String(sq) + " dBa");
}

String SIM7000G::getNetworkConnectionType() {
	ModemSerial.println("AT+CPSI?");
	delay(800);

	if (!ModemSerial.available()) {
		return String();
	}

	String result = ModemSerial.readString();
	result.replace("\r", "");
	result.replace("\n", "");
	result.replace("+CPSI: ", "");

	int commaIdx = result.indexOf(',');
	if (commaIdx < 0) {
		return String();
	}

	return result.substring(0, commaIdx);
}

String SIM7000G::getLocalIpAddress() {
	return modem.getLocalIP(); // AT+CIFSR
}


bool SIM7000G::sendSMS(const String recipient, const String message) {
	if (recipient.length() <= 0 || message.length() <= 0) {
		return false;
	}

	return modem.sendSMS(recipient, message); // AT+CMGS
}
