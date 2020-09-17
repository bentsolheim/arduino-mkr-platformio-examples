#include <Arduino.h>
#include <MKRNB.h>
#include <ArduinoBearSSL.h>

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

static const int ERROR_SSL_CONNECT = 1;
static const int ERROR_REQUEST_FAILED = 2;
static const int ERROR_RESPONSE_DOWNLOAD = 3;

int downloadSsl(const char *host, const char *path = "/");

bool gprsConnect(int attempts = 3, int delayMillis = 1000);

int copyStream(Stream *source, Stream *target, int waitForBytesAttempts = 10, int waitForBytesDelayMillis = 10);

bool performGet(Client *client, const char *host, const char *path);

void printModemVersion();

unsigned long getTime() {
    return nbAccess.getTime();
}

void setup() {
    // Let's pull the builtin led down to prevent it from floating...
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);

    Serial.begin(9600);
    while (!Serial) {}

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
    int status = downloadSsl("letsencrypt.org", "/");
    if (status != 0) {
        Serial.print("Unable to connect to host. Error: ");
        Serial.println(status);
    }
}

void loop() {
    yield();
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
 * Downloads https://${host}/${path} and prints it to Serial
 *
 * @param host
 * @param path
 * @return 0 on success, otherwise error code > 0
 */
int downloadSsl(const char *host, const char *path) {
    if (!sslClient.connect(host, 443)) {
        return ERROR_SSL_CONNECT;
    }
    if (!performGet(&sslClient, host, path)) {
        return ERROR_REQUEST_FAILED;
    }
    if (copyStream(&sslClient, &Serial) == -1) {
        return ERROR_RESPONSE_DOWNLOAD;
    }
    return 0;
}

/**
 * Sends a GET request for path the the specified host. The Connection header is set to close.
 *
 * @param client
 * @param host
 * @param path
 * @return true on success, false in case of error sending request
 */
bool performGet(Client *client, const char *host, const char *path) {

    char requestLine[300];
    snprintf(requestLine, sizeof(requestLine), "GET %s HTTP/1.1", path);
    if (client->println(requestLine) == 0) {
        return false;
    }
    snprintf(requestLine, sizeof(requestLine), "Host: %s", host);
    if (client->println(requestLine) == 0) {
        return false;
    }
    snprintf(requestLine, sizeof(requestLine), "Connection: close");
    if (client->println(requestLine) == 0) {
        return false;
    }
    client->println();
    return true;
}

/**
 * Copies bytes from source and prints them into target until there are no more bytes
 * available in source. When source runs out of bytes you can optionally wait a bit for more
 * data using waitForBytesAttempts and waitForBytesDelayMillis.
 *
 * @param source the Stream to read data from
 * @param target the Stream to print data to
 * @param waitForBytesAttempts the number of times to retry checking source for more data
 * @param waitForBytesDelayMillis the number of millis to wait before each retry attempt
 * @return the number of bytes copied, or -1 if any error during printing to target
 */
int copyStream(Stream *source, Stream *target, int waitForBytesAttempts, int waitForBytesDelayMillis) {
    char c;
    int bytesCopied = 0;
    for (int i = 0; i < waitForBytesAttempts; i++) {
        while (source->available()) {
            c = source->read();
            if (target->print(c) == 0) {
                return -1;
            }
            bytesCopied++;
            i = 0;
        }
        delay(waitForBytesDelayMillis);
    }
    return bytesCopied;
}
