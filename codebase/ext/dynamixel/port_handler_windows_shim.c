#if defined(_WIN32) || defined(_WIN64)
#define WINDLLEXPORT

#include "port_handler.h"
#include "..\..\serial.cpp"

SerialPort *serial;

int portHandlerWindows(const char *port_name) {

    char buffer[15];
    sprintf_s(buffer, sizeof(buffer), "\\\\.\\%s", port_name);
    serial = new SerialPort(buffer);

    g_used_port_num = 1;
    g_is_using = (uint8_t*)calloc(1, sizeof(uint8_t));

    return 0; // port_num
}

void closePortWindows(int port_num) {
    serial->closeSerial();
}

int readPortWindows(int, uint8_t *packet, int length) {
    serial->readSerialPort((const char *) packet, length);
    return 1;
}

int writePortWindows(int, uint8_t *packet, int length) {
    serial->writeSerialPort((const char *) packet, length);
    return 1;
}

void clearPortWindows(int port_num) { }
uint8_t openPortWindows(int port_num) { return 1; }
void setPortNameWindows(int port_num, const char *port_name) { }
char *getPortNameWindows(int port_num) { return 0; }
uint8_t setBaudRateWindows(int port_num, const int baudrate) { return 1; }
int getBaudRateWindows(int port_num) { return 0; }
void setPacketTimeoutWindows(int port_num, uint16_t packet_length) { }
void setPacketTimeoutMSecWindows(int port_num, double msec) { }
uint8_t isPacketTimeoutWindows(int port_num) { return 0; }
double getCurrentTimeWindows(int port_num) { return 0; }
double getTimeSinceStartWindows(int port_num) { return 0; }
uint8_t setupPortWindows(int port_num, const int baudrate) { return 0; }

#endif
