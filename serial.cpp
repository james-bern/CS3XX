/*
 * Author: Manash Kumar Mandal
 * Modified Library introduced in Arduino Playground which does not work
 * This works perfectly
 * LICENSE: MIT
 */

// TODO: Mac _serial header

#pragma once

#define ARDUINO_WAIT_TIME 128

#include <windows.h>
#include <iostream>

struct SerialPort {
    HANDLE handler;
    bool connected;
    COMSTAT status;
    DWORD errors;
    explicit SerialPort(const char *portName);
    ~SerialPort();

    int readSerialPort(const char *buffer, unsigned int buf_size);
    bool writeSerialPort(const char *buffer, unsigned int buf_size);
    bool isConnected();
    void closeSerial();
};


/*
struct {
    SerialPort *port;
} _serial;


void serial_init(int com_port_number) {
    char buffer[15];
    sprintf_s(buffer, sizeof(buffer), "\\\\.\\COM%d", com_port_number);
    _serial.port = new SerialPort(buffer);
}

bool serial_read(char *buffer, int buffer_size) {
    return (bool) _serial.port->readSerialPort(buffer, buffer_size);
}

void serial_write(char *buffer, int buffer_size = -1) {
    if (buffer_size == -1) { buffer_size = (int) strlen(buffer); }
    ASSERT(_serial.port->writeSerialPort((const char *) buffer, (unsigned int) buffer_size));
    return;
}

void serial_exit() {
    CloseHandle(_serial.port->handler);
}
*/









SerialPort::SerialPort(const char *portName) {
    this->connected = false;

    this->handler = CreateFileA(
            static_cast<LPCSTR>(portName),
            GENERIC_READ | GENERIC_WRITE,
            0,
            NULL,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            NULL);

    if (this->handler == INVALID_HANDLE_VALUE) {
        printf("[SerialPort] wrong COM port name?\n");
        return;
    }

    DCB dcbSerialParameters = {};
    ASSERT(GetCommState(this->handler, &dcbSerialParameters));

    dcbSerialParameters.BaudRate = 57600; // FORNOW! VERY IMPORTANT
    dcbSerialParameters.ByteSize = 8;
    dcbSerialParameters.StopBits = ONESTOPBIT;
    dcbSerialParameters.Parity = NOPARITY;
    dcbSerialParameters.fDtrControl = DTR_CONTROL_ENABLE;

    // ASSERT(SetupComm(handler, 4096, 4096)); // ?

    ASSERT(SetCommState(handler, &dcbSerialParameters));

    this->connected = true;
    PurgeComm(this->handler, PURGE_RXCLEAR | PURGE_TXCLEAR);
    Sleep(ARDUINO_WAIT_TIME);

    ASSERT(this->isConnected()); // try closing the Arduino _serial monitor
}










SerialPort::~SerialPort()
{
    if (this->connected)
    {
        this->connected = false;
        CloseHandle(this->handler);
    }
}

// Reading bytes from _serial port to buffer;
// returns read bytes count, or if error occurs, returns 0
int SerialPort::readSerialPort(const char *buffer, unsigned int buf_size)
{
    DWORD bytesRead{};
    unsigned int toRead = 0;

    ClearCommError(this->handler, &this->errors, &this->status);

    if (this->status.cbInQue > 0)
    {
        if (this->status.cbInQue > buf_size)
        {
            toRead = buf_size;
        }
        else
        {
            toRead = this->status.cbInQue;
        }
    }

    memset((void*) buffer, 0, buf_size);

    if (ReadFile(this->handler, (void*) buffer, toRead, &bytesRead, NULL))
    {
        return bytesRead;
    }

    return 0;
}

// Sending provided buffer to _serial port;
// returns true if succeed, false if not
bool SerialPort::writeSerialPort(const char *buffer, unsigned int buf_size)
{
    DWORD bytesSend;

    if (!WriteFile(this->handler, (void*) buffer, buf_size, &bytesSend, 0))
    {
        ClearCommError(this->handler, &this->errors, &this->status);
        return false;
    }

    return true;
}

// Checking if _serial port is connected
bool SerialPort::isConnected()
{
    if (!ClearCommError(this->handler, &this->errors, &this->status))
    {
        this->connected = false;
    }

    return this->connected;
}

void SerialPort::closeSerial()
{
    CloseHandle(this->handler);
}
