# tiny-gateway

A WIP lightweight SMS gateway for the LILYGO T-SIM7000G. It allows sending SMS
messages via a simple HTTP API.

Currently, the API can only be reached via the WiFi access point running on the
ESP32 at `192.168.4.1`.

### Hardware requirements
- LILYGO T-SIM7000G
- Nano SIM card with SMS plan
- Optional: 18650 battery

### API usage

**Send SMS**
```
POST /send-sms
Authorization: Bearer <HTTP_AUTH_TOKEN>
Content-Type: text/plain

{
	"recipient": "<PHONE_NUMBER>",
	"message": "<MESSAGE>"
}
```

Or in Bash using curl:
```
curl http://192.168.4.1/send-sms \
	-X POST \
	-H 'Authorization: Bearer <HTTP_AUTH_TOKEN>' \
	-H 'Content-Type: text/plain' \
	--data '{"recipient":"<PHONE_NUMBER>","message":"<MESSAGE>"}'
```

### Configuration

In the `globals.h` file, update this line with your APN:
```cpp
#define APN (char*)"example-apn.com"
```

Optionally, the WiFi access point credentials can also be configured:
```cpp
#define AP_SSID     (char*)"TinyGateway"
#define AP_PASSWORD (char*)"hl54b6xwp2n6rxey"
```
