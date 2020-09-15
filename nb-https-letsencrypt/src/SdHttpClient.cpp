#include <Client.h>
#include <Arduino.h>
#include "SdHttpClient.h"

int prepareGetRequest(Client *client, const char *host, const char *path) {

    char requestLine[300];
    snprintf(requestLine, sizeof(requestLine), "GET %s HTTP/1.1", path);
    Serial.println(requestLine);
    if (client->println(requestLine) == 0) {
        return 0;
    }
    snprintf(requestLine, sizeof(requestLine), "Host: %s", host);
    Serial.println(requestLine);
    if (client->println(requestLine) == 0) {
        return 0;
    }
    snprintf(requestLine, sizeof(requestLine), "Accept: */*");
    Serial.println(requestLine);
    if (client->println(requestLine) == 0) {
        return 0;
    }
    snprintf(requestLine, sizeof(requestLine), "Connection: close");
    Serial.println(requestLine);
    if (client->println(requestLine) == 0) {
        return 0;
    }
    client->println();
    client->flush();
    return 1;
}

int downloadToStream(Client *client, Stream *target, int waitForBytesAttempts, int waitForBytesDelayMillis) {

    int contentLength, status;
    contentLength = status = readHeaderSection(client);
    if (status == -1) {
        return ERROR_HTTP_READ_ERROR;
    } else if (status == 0) {
        return ERROR_CONTENT_LENGTH_UNKNOWN;
    }

    int bytesCopied = copyStream(client, target, waitForBytesAttempts, waitForBytesDelayMillis);
    if (bytesCopied == -1) {
        return ERROR_FILE_WRITE;
    }

    target->flush();
    if (status < -1) {
        return status;
    }
    if (bytesCopied != contentLength) {
        return ERROR_CONTENT_LENGTH_MISMATCH;
    }

    return DOWNLOAD_OK;
}

int getStatusCode(char httpStatusLine[]) {
    // HTTP/1.1 404 Not Found
    char *statusCode = httpStatusLine;
    while (statusCode[0] != ' ') statusCode++;

    return strtol(statusCode, nullptr, 10);
}

/**
 * Reads the HTTP header section from the client and stops right before the response body starts. Will return the
 * value of the Content-Type header if it has been set. 0 otherwise. Returns -1 on any errors reading the http stream,
 * and in case of non 200 HTTP status code the status code multiplied by -1 is returned.
 * The method assumes that no bytes have been previously read from the response.
 * @param client
 * @return
 */
int readHeaderSection(Client *client) {

    if (!client->connected()) {
        return -1;
    }
    int contentLength = 0;
    int statusCode = 0;

    char line[1000]; // Have not figured out how to make the line length arbitrary
    while (true) {
        size_t readBytes = client->readBytesUntil('\n', line, sizeof(line));
        if (readBytes == 0) {
            return -1;
        }
        if (readBytes == 1 && line[0] == '\r') {
            // We have encountered a line with only \r\n, which indicates end of http header section
            break;
        }
        line[readBytes - 1] = '\0'; // This will overwrite the \r preceding the \n char - we don't need that
        Serial.println(line);

        int headerNameEndIndex = 0;
        while (headerNameEndIndex < readBytes && line[headerNameEndIndex] != ':') headerNameEndIndex++;

        if (readBytes == headerNameEndIndex) {
            // This line does not contain a ':', which means this is the http status line.
            statusCode = getStatusCode(line);
            continue;
        }

        // Get the http header name from the line
        char headerName[headerNameEndIndex + 1];
        for (int i = 0; i < headerNameEndIndex; i++) {
            headerName[i] = tolower(line[i]);
        }
        headerName[headerNameEndIndex] = '\0';

        // Get the http header value from the line
        char value[readBytes - headerNameEndIndex];
        int valueStartIndex = headerNameEndIndex + 1; // The header name ends at ':'. Start right after that.
        while (line[valueStartIndex] == ' ') valueStartIndex++; // Skip any white spaces after ':'
        for (int i = valueStartIndex; i < readBytes; i++) {
            value[i - valueStartIndex] = line[i];
        }
        value[readBytes - valueStartIndex] = '\0';

        if (strcmp(headerName, "content-length") == 0) {
            contentLength = atoi(value);
        }
    }
    if (statusCode != 200) {
        return statusCode * -1;
    }
    return contentLength;
}

int copyStream(Stream *source, Stream *target, int waitForBytesAttempts, int waitForBytesDelayMillis) {
    char c;
    int bytesCopied = 0;
    for (int i=0; i<waitForBytesAttempts; i++) {
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
