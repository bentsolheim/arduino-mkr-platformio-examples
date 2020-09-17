# nb-https-letsencrypt

## Description

When I got the Arduino MKR NB 1500 from https://www.elfadistrelec.no/ it came with the U-Blox SARA-R410M-02B modem with 
firmware/app version L0.0.00.00.05.06,A.02.00. Although the MKRNB@1.3.2 library will load a lot of commonly used
SSL/TLS certificates, a lot of connections to servers using these certificates will still fail. I believe this has 
to do with SSL/TLS version used by some servers not being supported by the U-Blox modem. The modem firmware and app
version is not the newest available, but the process for accessing new firmware and updating the modem on the MRK boards
is convoluted and a bit tricky, and I don't even know for sure if updating the firmware will solve the problem.

To allow the MKR boards to make general use of https and access most servers, you can use the ArduinoBearSSL library
for SSL/TLS instead of the implementation on the modem. This example shows how.

The example (like all examples in this repo) is intended to be run with https://platformio.org so you will have to 
install that first. Then create the `src/arduino_secrets.h` file based on the `src/default_secrets.h` file, connect 
your Arduino MKR NB 1500 and run

    pio run -t upload
    
The example will wait for you to connect to serial to observe the output.