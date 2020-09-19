#include "http_ssl.h"

/**
 * Downloads https://${host}/${path} and prints it to Serial
 *
 * @param host
 * @param path
 * @return 0 on success, otherwise error code > 0
 */
int downloadSsl(Client *sslClient, const char *host, const char *path) {
    if (!sslClient->connect(host, 443)) {
        return ERROR_SSL_CONNECT;
    }
    if (!performGet(sslClient, host, path)) {
        return ERROR_REQUEST_FAILED;
    }
    if (copyStream(sslClient, &Serial) == -1) {
        return ERROR_RESPONSE_DOWNLOAD;
    }
    return 0;
}

/**
 * Sends a GET request for the path of the specified host. The Connection header is set to close.
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

