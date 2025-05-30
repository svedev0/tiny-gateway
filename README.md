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
Content-Type: text/plain

Recipient=+123456789
Test message
```

Or in Bash using curl:
```
curl http://192.168.4.1/send-sms -X POST -H 'Content-Type: text/plain' --data $'Recipient=+123456789\nTest message'
```

### Configuration

In the `globals.h` file, update this line with your APN:
```cpp
#define APN (char*)"example-apn.com"
```

Also in the `globals.h` file, update these lines to configure the WiFi access
point credentials:
```cpp
#define AP_SSID     (char*)"SIM7000G AP"
#define AP_PASSWORD (char*)"aptest123"
```
