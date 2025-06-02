#include "globals.h"
#include "httpserver.h"

HttpServer::HttpServer(const int port) {
	WiFiServer server(port);
	this->port = port;
	this->server = server;
	delay(100);
}

void HttpServer::begin() {
	server.begin();
}

bool HttpServer::available() {
	return server.available();
}

bool HttpServer::serve(String& header, String& body, const bool debug) {
	WiFiClient client = server.available();
	if (!client) {
		header = String();
		body = String();
		return false;
	}

	uint32_t currMs = millis();
	uint32_t prevMs = currMs;
	const uint32_t timeoutMs = 2000;

	String request = "";
	int contentLength = 0;
	bool headerEnded = false;

	while (client.connected() && currMs - prevMs <= timeoutMs) {
		currMs = millis();

		while (client.available()) {
			request += (char)client.read();

			if (headerEnded || !request.endsWith("\r\n\r\n")) {
				continue;
			}

			headerEnded = true;

			// Get Content-Length header (if any)
			int index = request.indexOf("Content-Length: ");
			if (index != -1) {
				int start = index + 16;
				int end = request.indexOf("\r\n", start);
				if (end != -1) {
					contentLength = request.substring(start, end).toInt();
				}
			}

			break;
		}

		// Read body (if any)
		if (headerEnded && contentLength > 0) {
			int bytesRead = 0;
			while (client.available() && bytesRead < contentLength) {
				request += (char)client.read();
				bytesRead++;
			}

			if (bytesRead >= contentLength) {
				break;
			}
		}

		if (headerEnded && contentLength == 0) {
			break;
		}
	}

	int endOfHeader = request.indexOf("\r\n\r\n");
	header = request.substring(0, endOfHeader);
	header.trim();

	body = request.substring(endOfHeader);
	body.trim();

	if (debug) {
		UsbSerial.println(request);
	}

	const char* resHeader =
		"HTTP/1.1 200 OK\r\n"
		"Content-Type: text/plain\r\n"
		"Connection: close\r\n"
		"\r\n"
		"Request received";
	client.println(resHeader);

	delay(5);
	client.stop();

	return true;
}
