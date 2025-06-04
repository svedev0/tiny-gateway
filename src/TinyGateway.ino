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
	// Start USB serial.
	UsbSerial.begin(USB_UART_BAUD);
	while (!UsbSerial) {
		delay(5);
	}
	UsbSerial.println("---");
	delay(100);
	uint32_t startTime = millis();

	log("(i) Powering on peripherals...");
	// Turn off ESP32 LED.
	pinMode(LED_PIN, OUTPUT);
	digitalWrite(LED_PIN, HIGH);

	// Power on modem.
	pinMode(MODEM_POWER_PIN, OUTPUT);
	digitalWrite(MODEM_POWER_PIN, HIGH);
	delay(100);
	digitalWrite(MODEM_POWER_PIN, LOW);
	delay(500);

	// Start modem serial.
	ModemSerial.begin(MODEM_UART_BAUD, MODEM_UART_CFG, MODEM_UART_RX_PIN, MODEM_UART_TX_PIN);
	while (!ModemSerial) {
		delay(5);
	}
	delay(500);

	log("(i) Initialising modem...");
	if (!I_KNOW_WHAT_I_AM_DOING) {
		// Reboot and initialise modem.
		if (!modem.restart()) {
			logFatal("(-) Failed to restart modem!");
		}
		delay(800);
	}

	// Read modem hardware information.
	if (VERBOSE_LOGGING) {
		log("(i) Modem manufacturer: " + modem.getHardwareManufacturer());
		log("(i) Modem model: " + modem.getHardwareModel());
		log("(i) Modem firmware version: " + modem.getFirmwareVersion());
	}

	// Configure modem.
	if (!I_KNOW_WHAT_I_AM_DOING) {
		log("(i) Setting network mode...");
		if (!modem.setNetworkMode(38, false)) {
			logFatal("(-) Failed to set network mode!");
		}

		log("(i) Setting radio mode...");
		if (!modem.setRadioMode(1, false)) {
			logFatal("(-) Failed to set radio mode!");
		}

		log("(i) Setting PDP parameters...");
		if (!modem.setPDPParams()) {
			logFatal("(-) Failed to set PDP parameters!");
		}
	}

	log("(i) Unlocking SIM PIN...");
	if (!modem.unlockSIM()) {
		logFatal("(-) Failed to unlock SIM PIN!");
	}

	log("(i) Connecting to network...");
	if (!modem.connectToNetwork(10000UL)) {
		logFatal("(-) Failed to connect to network!");
	}

	log("(+) Modem ready");

	// Read modem network information.
	if (VERBOSE_LOGGING) {
		log("(i) IMEI: " + modem.getIMEI());
		log("(i) IMSI: " + modem.getIMSI());
		log("(i) ICCID: " + modem.getICCID());
		log("(i) Phone number: " + modem.getPhoneNumber());
		log("(i) Operator: " + modem.getNetworkOperator());
		log("(i) Signal quality: " + modem.getNetworkSignalQuality());
		log("(i) Connection type: " + modem.getNetworkConnectionType());
		log("(i) Local IP: " + modem.getLocalIpAddress());
	}

	// Configure and start WiFi access point.
	log("(i) Initialising WiFi access point...");
	WiFi.mode(WIFI_AP);
	if (!WiFi.softAP(AP_SSID, AP_PASSWORD)) {
		logFatal("(-) Failed to start access point!");
	}
	while (WiFi.softAPIP().toString() == "0.0.0.0") {
		delay(50);
	}
	log("(+) WiFi access point '" + String(AP_SSID) + "' started");
	delay(200);

	// Configure and start HTTP server.
	log("(i) Starting HTTP server...");
	server.on("/send-sms", handleSendSMS);
	server.onNotFound(handleNotFound);
	server.begin();
	String serverIp = WiFi.softAPIP().toString();
	log("(+) HTTP server listening on 'http://" + serverIp + "/'");

	uint32_t elapsedTime = millis() - startTime;
	log("(+) Setup completed in " + String(elapsedTime) + " ms");
}

void loop() {
	modem.maintain();
	server.handleClient();
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

// Handle send SMS requests.
static void handleSendSMS() {
	if (server.method() != HTTP_POST) {
		log("(-) Invalid request, method is not POST!");
		server.send(405, "text/plain", "405 Method Not Allowed\r\n");
		return;
	}

	if (!requestAuthorized()) {
		log(lastError);
		server.send(401, "text/plain", "401 Unauthorized\r\n");
		return;
	}

	String reqBody = server.arg("plain");
	JsonDocument json;
	if (!parseRequestBody(reqBody, json)) {
		log(lastError);
		server.send(400, "text/plain", "400 Bad Request\r\n");
		return;
	}

	String recipient = json["recipient"];
	String message = json["message"];
	log("(i) Sending SMS to '" + recipient + "'...");

	if (modem.sendSMS(recipient, message)) {
		log("(+) SMS sent successfully");
		server.send(200, "text/plain", "200 OK\r\n");
	}
	else {
		log("(-) Failed to send SMS!");
		server.send(400, "text/plain", "400 Bad Request\r\n");
	}
}

// Handle not found requests.
static void handleNotFound() {
	server.send(404, "text/plain", "404 Not Found\r\n");
}

// Utility functions ===============================================================

// Print fatal error and halt execution.
static void logFatal(const String& message) {
	UsbSerial.println(message);
	UsbSerial.println("(-) Fatal error, please restart device!");
	while (true) {
		delay(1000);
	}
}

// Print message.
static void log(const String& message) {
	if (VERBOSE_LOGGING) {
		UsbSerial.println(message);
	}
	else if (!message.startsWith("(i)")) {
		UsbSerial.println(message);
	}
	delay(50); // Allow time for USB serial to process.
}
