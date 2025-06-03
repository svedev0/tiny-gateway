#include <ArduinoJson.h>
#include <WebServer.h>
#include <WiFi.h>
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

	// Initialise components.
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

	// Read modem hardware information.
	String manufacturer = modem.getHardwareManufacturer();
	UsbSerial.println("(i) Modem manufacturer: " + manufacturer);

	String model = modem.getHardwareModel();
	UsbSerial.println("(i) Modem model: " + model);

	String fwVersion = modem.getFirmwareVersion();
	UsbSerial.println("(i) Modem firmware version: " + fwVersion);

	// Configure modem.
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

	// Read modem network information.
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

	// Configure HTTP server.
	server.on("/send-sms", []() {
		if (server.method() != HTTP_POST) {
			UsbSerial.println("(-) Invalid request, method is not POST!");
			server.send(405, "text/plain", "405 Method Not Allowed\r\n");
			return;
		}

		if (!requestAuthorized()) {
			UsbSerial.println(lastError);
			server.send(401, "text/plain", "401 Unauthorized\r\n");
			return;
		}

		String reqBody = server.arg("plain");
		JsonDocument json;
		if (!parseRequestBody(reqBody, json)) {
			UsbSerial.println(lastError);
			server.send(400, "text/plain", "400 Bad Request\r\n");
			return;
		}

		String recipient = json["recipient"];
		String message = json["message"];
		UsbSerial.println("(i) Sending SMS to '" + recipient + "'...");

		if (modem.sendSMS(recipient, message)) {
			UsbSerial.println("(+) SMS sent successfully");
			server.send(200, "text/plain", "200 OK\r\n");
		}
		else {
			UsbSerial.println("(-) Failed to send SMS!");
			server.send(400, "text/plain", "400 Bad Request\r\n");
		}
		});

	server.onNotFound([]() {
		server.send(404, "text/plain", "404 Not Found\r\n");
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

// Deserialize JSON request body.
static bool parseRequestBody(const String& reqBody, JsonDocument& doc) {
	DeserializationError error = deserializeJson(doc, reqBody);
	if (error) {
		lastError = "(-) Failed to parse request body: " + String(error.c_str());
		doc = JsonDocument();
		return false;
	}

	return true;
}
