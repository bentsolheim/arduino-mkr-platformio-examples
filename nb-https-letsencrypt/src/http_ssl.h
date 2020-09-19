#include <Arduino.h>
#include <ArduinoBearSSL.h>

#ifndef HTTPS_LETSENCRYPT_GPRS_SSL_H
#define HTTPS_LETSENCRYPT_GPRS_SSL_H

static const int ERROR_SSL_CONNECT = 1;
static const int ERROR_REQUEST_FAILED = 2;
static const int ERROR_RESPONSE_DOWNLOAD = 3;

int downloadSsl(Client *client, const char *host, const char *path = "/");

int copyStream(Stream *source, Stream *target, int waitForBytesAttempts = 10, int waitForBytesDelayMillis = 10);

bool performGet(Client *client, const char *host, const char *path);

#endif //HTTPS_LETSENCRYPT_GPRS_SSL_H
