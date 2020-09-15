#ifndef MEM_SHIELD_OTA_SDHTTPCLIENT_H
#define MEM_SHIELD_OTA_SDHTTPCLIENT_H

#include <Client.h>


static const int DOWNLOAD_OK = 0;
static const int ERROR_HTTP_CONNECTION_FAILED = -1;
static const int ERROR_HTTP_READ_ERROR = -4;
static const int ERROR_FILE_WRITE = -2;
static const int ERROR_CONTENT_LENGTH_MISMATCH = -5;
static const int ERROR_CONTENT_LENGTH_UNKNOWN = -6;

int downloadToStream(Client *client, Stream *target, int waitForBytesAttempts = 10, int waitForBytesDelayMillis = 10);

int readHeaderSection(Client *client);

int copyStream(Stream *source, Stream *target, int waitForBytesAttempts = 10, int waitForBytesDelayMillis = 10);

int prepareGetRequest(Client *client, const char *host, const char *path);

#endif //MEM_SHIELD_OTA_SDHTTPCLIENT_H
