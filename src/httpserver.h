#ifndef _HTTPSERVER_h
#define _HTTPSERVER_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
#include "WProgram.h"
#endif // defined(ARDUINO) && ARDUINO >= 100

#include <WiFi.h>

class HttpServer {
private:
	int port;
	WiFiServer server;

public:
	// Creates a new HTTP server on the specified port.
	HttpServer(int port);

	// Starts the HTTP server.
	void begin();

	// Returns true if the server is running.
	bool available();

	// Accepts a client connection and serves the request. Return true if successful.
	bool serve(String& header, String& body, bool debug);
};

#endif // _HTTPSERVER_h
