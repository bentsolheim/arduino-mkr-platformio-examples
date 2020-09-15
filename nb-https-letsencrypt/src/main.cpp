#include <Arduino.h>
#include <MKRNB.h>
#include <Wire.h>
#include <BearSSLClient.h>
#include <ArduinoBearSSL.h>
#include "SdHttpClient.h"

#if __has_include("arduino_secrets.h")

#include "arduino_secrets.h"

#else
#include "default_secrets.h"
#endif


const char PINNUMBER[] = SECRET_PINNUMBER;

NBClient client;
BearSSLClient sslClient(client);
GPRS gprs;
NB nbAccess(false);
NBScanner nbScanner;

void sslConnect(const char *host, const char *path="/");

void gprsConnect();

String response;

unsigned long getTime() {
    return nbAccess.getTime();
}

void setup() {
    Serial.begin(9600);
    while (!Serial) { ; // wait for serial port to connect. Needed for native USB port only
    }
    Serial.println("Starting");

    gprsConnect();

    ArduinoBearSSL.onGetTime(getTime);
}

void loop() {

    Serial.println("\n\n\n");
    delay(5000);

    MODEM.send("AT+GMI");
    MODEM.waitForResponse(100, &response);
    Serial.println(response);

    MODEM.send("AT+GMM");
    MODEM.waitForResponse(100, &response);
    Serial.println(response);

    MODEM.send("AT+GMR");
    MODEM.waitForResponse(100, &response);
    Serial.println(response);

    MODEM.send("ATI9");
    MODEM.waitForResponse(100, &response);
    Serial.println(response);

    Serial.print("GPRS ready: ");
    Serial.println(gprs.ready());

    Serial.print("GPRS status: ");
    Serial.println(gprs.status());

    Serial.print("NB access ready: ");
    Serial.println(nbAccess.ready());

    Serial.print("NB access status: ");
    Serial.println(nbAccess.status());

    Serial.print("NB access alive: ");
    Serial.println(nbAccess.isAccessAlive());

    Serial.print("NB scanner carrier: ");
    Serial.println(nbScanner.getCurrentCarrier());

    Serial.print("NB scanner signal: ");
    Serial.println(nbScanner.getSignalStrength());

    Serial.println("\n\n");
    sslConnect("letsencrypt.org");
}

void sslConnect(const char *host, const char *path) {
    unsigned long start = millis();
    Serial.print("Start connect ");
    Serial.print(host);
    Serial.print(" ... ");
    bool connected = sslClient.connect(host, 443);
    Serial.println("connected");
    if (!connected) {
        Serial.print("Unable to connect to ");
        Serial.println(host);
    } else {
        Serial.print("Connected to ");
        Serial.print(host);
        Serial.print(" in ");
        Serial.print(millis() - start);
        Serial.println(" millis");

        Serial.println("Prepare");
        Serial.println(prepareGetRequest(&sslClient, host, path));
        Serial.println("Download");
        int status = downloadToStream(&sslClient, &Serial);
        Serial.println();
        Serial.println(status);
    }
}

void gprsConnect() {
    bool connected = false;
    while (!connected) {
        if (nbAccess.begin(PINNUMBER, SECRET_GPRS_APN, SECRET_GPRS_LOGIN, SECRET_GPRS_PASSWORD) == NB_READY) {
            connected = true;
        } else {
            Serial.println("Not connected");
            delay(1000);
        }
    }

    Serial.println("Connected to the network");
}
