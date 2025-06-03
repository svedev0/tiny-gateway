#include <WiFi.h>
#include <WebServer.h>
#include "globals.h"
#include "modem.h"

SIM7000G modem(ModemSerial);
WebServer server(HTTP_PORT);
String lastError = "";

// Arduino functions ===============================================================

void setup() {
	UsbSerial.begin(USB_UART_BAUD);
	delay(1000);
	UsbSerial.println("");
	UsbSerial.println("(i) Booting...");

	// Initialise components
	UsbSerial.println("(i) Powering on peripherals...");
	powerOnPeripherals();
	delay(500);

	UsbSerial.println("(i) Initialising WiFi access point...");
	while (!WiFi.softAP(AP_SSID, AP_PASSWORD)) {
		UsbSerial.println("(-) Failed to start access point!");
		delay(500);
	}
	while (WiFi.softAPIP().toString() == "0.0.0.0") {
		delay(100);
	}
	UsbSerial.println("(+) WiFi access point created");
	UsbSerial.println("(i) WiFi IP address: " + WiFi.softAPIP().toString());
	delay(200);

	UsbSerial.println("(i) Initialising modem...");
	if (!bootModem()) {
		UsbSerial.println("(-) Failed to restart modem, attempting to continue without restarting!");
	}
	delay(500);

	// Read modem information
	String manufacturer = modem.getHardwareManufacturer();
	UsbSerial.println("(i) Modem manufacturer: " + manufacturer);

	String model = modem.getHardwareModel();
	UsbSerial.println("(i) Modem model: " + model);

	String fwVersion = modem.getFirmwareVersion();
	UsbSerial.println("(i) Modem firmware version: " + fwVersion);

	// Configure modem
	UsbSerial.println("(i) Setting network mode...");
	if (!modem.setNetworkMode(38, false)) {
		UsbSerial.println("(-) Failed to set network mode!");
	}

	UsbSerial.println("(i) Setting radio mode...");
	if (!modem.setRadioMode(1, false)) {
		UsbSerial.println("(-) Failed to set radio mode!");
	}

	UsbSerial.println("(i) Setting PDP parameters...");
	if (!modem.setPDPParams()) {
		UsbSerial.println("(-) Failed to set PDP parameters!");
	}

	UsbSerial.println("(i) Setting equipment functionality mode...");
	if (!modem.setUEFunctionality(1)) {
		UsbSerial.println("(-) Failed to set equipment functionality mode!");
	}

	UsbSerial.println("(i) Unlocking SIM PIN...");
	if (!modem.unlockSIM()) {
		UsbSerial.println("(-) Failed to unlock SIM PIN!");
	}

	UsbSerial.println("(i) Connecting to network...");
	if (!modem.connectToNetwork(10000UL)) {
		UsbSerial.println("(-) Failed to validate network!");
	}

	// Read network information
	String imei = modem.getIMEI();
	UsbSerial.println("(i) IMEI: " + imei);

	String imsi = modem.getIMSI();
	UsbSerial.println("(i) IMSI: " + imsi);

	String phoneNum = modem.getPhoneNumber();
	UsbSerial.println("(i) Phone number: " + phoneNum);

	String netOp = modem.getNetworkOperator();
	UsbSerial.println("(i) Operator: " + netOp);

	String netSigQual = modem.getNetworkSignalQuality();
	UsbSerial.println("(i) Signal quality: " + netSigQual);

	String netConnType = modem.getNetworkConnectionType();
	UsbSerial.println("(i) Connection type: " + netConnType);

	String localIp = modem.getLocalIpAddress();
	UsbSerial.println("(i) Local IP: " + localIp);

	// Configure HTTP server
	server.on("/send-sms", HTTP_POST, []() {
		if (!requestAuthorized()) {
			UsbSerial.println(lastError);
			server.send(401, "text/plain", "Unauthorized\r\n");
			return;
		}

		String reqBody = server.arg("plain");
		String recipient;
		String message;
		if (!parseSmsParams(reqBody, recipient, message)) {
			UsbSerial.println(lastError);
			server.send(400, "text/plain", "Invalid request\r\n");
			return;
		}

		UsbSerial.println("(i) Sending SMS to '" + recipient + "'...");
		if (modem.sendSMS(recipient, message)) {
			UsbSerial.println("(+) SMS sent successfully");
			server.send(200, "text/plain", "Request received\r\n");
		}
		else {
			UsbSerial.println("(-) Failed to send SMS!");
			server.send(400, "text/plain", "Invalid request\r\n");
		}
		});

	server.onNotFound([]() {
		server.send(404, "text/plain", "Not found\r\n");
		});

	UsbSerial.println("(i) Starting HTTP server...");
	server.begin();
}

void loop() {
	modem.maintain();
	server.handleClient();
}

// Init functions ==================================================================

// Power on peripherals.
static void powerOnPeripherals() {
	pinMode(LED_PIN, OUTPUT);
	digitalWrite(LED_PIN, HIGH);

	pinMode(MODEM_POWER_PIN, OUTPUT);
	digitalWrite(MODEM_POWER_PIN, HIGH);
	delay(100);
	digitalWrite(MODEM_POWER_PIN, LOW);
}

// Initialise modem.
static bool bootModem() {
	ModemSerial.begin(
		MODEM_UART_BAUD,
		MODEM_UART_CFG,
		MODEM_UART_RX_PIN,
		MODEM_UART_TX_PIN);
	delay(500);
	return modem.restart();
}

// HTTP server functions ===========================================================

// Returns true if request is authorized.
static bool requestAuthorized() {
	if (!server.hasHeader("Authorization")) {
		lastError = "(-) Invalid request, missing authorization header!";
		return false;
	}

	String authHeader = server.header("Authorization");
	if (authHeader.length() <= 0) {
		lastError = "(-) Invalid request, authorization header is empty!";
		return false;
	}

	if (!authHeader.startsWith("Bearer ")) {
		lastError = "(-) Invalid request, authorization header is not Bearer!";
		return false;
	}

	String token = authHeader.substring(7); // Remove "Bearer " prefix
	if (token.length() <= 0) {
		lastError = "(-) Invalid request, token is empty!";
		return false;
	}

	if (token != HTTP_AUTH_TOKEN) {
		lastError = "(-) Invalid request, token is invalid!";
		return false;
	}

	return true;
}

// Parse SMS parameters from request body.
static bool parseSmsParams(const String& reqBody, String& recipient, String& message) {
	recipient = String();
	message = String();

	if (reqBody.length() <= 0) {
		lastError = "(-) Invalid request, body is empty!";
		return false;
	}

	if (!reqBody.startsWith("Recipient=")) {
		lastError = "(-) Invalid request, recipient key missing!";
		return false;
	}

	int lineEnd = reqBody.indexOf('\n');
	if (lineEnd < 0) {
		lastError = "(-) Invalid request, no newline found!";
		return false;
	}

	recipient = reqBody.substring(10, lineEnd);
	message = reqBody.substring(lineEnd + 1);
	return true;
}
