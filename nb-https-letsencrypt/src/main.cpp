#include <Arduino.h>
#include <MKRNB.h>
#include <ArduinoBearSSL.h>
#include "http_ssl.h"

// Create the arduino_secrets.h file with the specifics for connecting to the network for
// your provider.
#if __has_include("arduino_secrets.h")

#include "arduino_secrets.h"

#else
#include "default_secrets.h"
#endif

const char PINNUMBER[] = SECRET_PINNUMBER;
const char GPRS_APN[] = SECRET_GPRS_APN;
const char GPRS_LOGIN[] = SECRET_GPRS_LOGIN;
const char GPRS_PASSWORD[] = SECRET_GPRS_PASSWORD;

NB nbAccess(false);
NBClient nbClient;

// Create a BearSSLClient object, wrapping the NBClient. This is what will allow us to use
// https connections without relying on the NBSSLClient in the MKRNB library.
BearSSLClient sslClient(nbClient);

bool gprsConnect(int attempts=3, int delayMillis=1000);
void printModemVersion();

unsigned long getTime() {
    return nbAccess.getTime();
}

void setup() {
    // Let's pull the builtin led down to prevent it from floating...
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);

    Serial.begin(9600);
    while (!Serial) { delay(100); }

    Serial.println("Connecting to network...");

    if (!gprsConnect()) {
        Serial.print("Unable to connect to network. Aborting.");
        return;
    }
    Serial.println("Connected to network successfully...");

    printModemVersion();

    // Important step. Let BearSSL know how to get the current time.
    // Needed for certificate verification.
    ArduinoBearSSL.onGetTime(getTime);

    // Download https://letsencrypt.org . This will not work using NBSSLClient on the
    // u-blox SARA-R410M-02B modem with firmware/app version L0.0.00.00.05.06,A.02.00
    // from Feb 03 2018. BearSSL to the rescue.
    int status = downloadSsl(&sslClient, "letsencrypt.org", "/");
    if (status != 0) {
        Serial.print("Unable to connect to host. Error: ");
        Serial.println(status);
    }
}

void loop() {
    // We are not doing anything besides what is done in setup().
    yield();
}

/**
 * Connects to the network.
 *
 * @param attempts the number of times to retry in case of failure
 * @param delayMillis the number of milliseconds to wait before each retry
 * @return true on connection success, false otherwise
 */
bool gprsConnect(int attempts, int delayMillis) {
    for (int i = 0; i < attempts; i++) {
        if (nbAccess.begin(PINNUMBER, GPRS_APN, GPRS_LOGIN, GPRS_PASSWORD) == NB_READY) {
            return true;
        } else {
            delay(delayMillis);
        }
    }
    return false;
}

/**
 * Prints the modem name and version. Do not call this method before MODEM.begin() has been
 * called, or the first send call with hang forever.
 */
void printModemVersion() {
    String response;

    MODEM.send("AT+GMM");
    MODEM.waitForResponse(100, &response);
    Serial.print(response);
    Serial.print(" version ");

    MODEM.send("ATI9");
    MODEM.waitForResponse(100, &response);
    Serial.println(response);
}
