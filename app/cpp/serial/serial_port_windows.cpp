#include "serial_port.h"
#include <windows.h>
#include <string>

static HANDLE hComm = INVALID_HANDLE_VALUE;

SerialPort::SerialPort()
    : fd_(-1), timeout_ms_(1000) {}

SerialPort::~SerialPort() {
    close();
}

bool SerialPort::isOpen() const {
    return hComm != INVALID_HANDLE_VALUE;
}

SerialError SerialPort::open(const SerialConfig& cfg) {
    timeout_ms_ = cfg.timeout_ms;

    // COM10 이상 대응
    std::string portName = cfg.port;
    if (portName.rfind("COM", 0) == 0) {
        portName = "\\\\.\\" + portName;
    }

    hComm = CreateFileA(
        portName.c_str(),
        GENERIC_READ | GENERIC_WRITE,
        0,
        NULL,
        OPEN_EXISTING,
        0,
        NULL
    );

    if (hComm == INVALID_HANDLE_VALUE)
        return SerialError::PORT_OPEN_FAIL;

    // -------- DCB 설정 --------
    DCB dcb{};
    dcb.DCBlength = sizeof(DCB);

    if (!GetCommState(hComm, &dcb)) {
        close();
        return SerialError::CONFIG_FAIL;
    }

    dcb.BaudRate = cfg.baudrate;
    dcb.ByteSize = static_cast<BYTE>(cfg.databits);

    // Stop bits
    dcb.StopBits = (cfg.stopbits == 2) ? TWOSTOPBITS : ONESTOPBIT;

    // Parity
    if (cfg.parity == 'E')
        dcb.Parity = EVENPARITY;
    else if (cfg.parity == 'O')
        dcb.Parity = ODDPARITY;
    else
        dcb.Parity = NOPARITY;

    dcb.fParity = (cfg.parity != 'N');

    // Flow control OFF
    dcb.fOutxCtsFlow = FALSE;
    dcb.fOutxDsrFlow = FALSE;
    dcb.fDtrControl  = DTR_CONTROL_DISABLE;
    dcb.fRtsControl  = RTS_CONTROL_DISABLE;
    dcb.fOutX        = FALSE;
    dcb.fInX         = FALSE;

    if (!SetCommState(hComm, &dcb)) {
        close();
        return SerialError::CONFIG_FAIL;
    }

    // -------- Timeout 설정 --------
    COMMTIMEOUTS timeouts{};
    timeouts.ReadIntervalTimeout         = timeout_ms_;
    timeouts.ReadTotalTimeoutConstant    = timeout_ms_;
    timeouts.ReadTotalTimeoutMultiplier  = 0;
    timeouts.WriteTotalTimeoutConstant   = timeout_ms_;
    timeouts.WriteTotalTimeoutMultiplier = 0;

    if (!SetCommTimeouts(hComm, &timeouts)) {
        close();
        return SerialError::CONFIG_FAIL;
    }

    // Buffer clear
    PurgeComm(hComm, PURGE_RXCLEAR | PURGE_TXCLEAR);

    return SerialError::NONE;
}

void SerialPort::close() {
    if (hComm != INVALID_HANDLE_VALUE) {
        CloseHandle(hComm);
        hComm = INVALID_HANDLE_VALUE;
    }
}

SerialError SerialPort::write(const uint8_t* data, size_t len) {
    if (hComm == INVALID_HANDLE_VALUE)
        return SerialError::PORT_NOT_OPEN;

    DWORD written = 0;
    if (!WriteFile(hComm, data, static_cast<DWORD>(len), &written, NULL))
        return SerialError::WRITE_FAIL;

    if (written != len)
        return SerialError::WRITE_FAIL;

    return SerialError::NONE;
}

SerialError SerialPort::read(uint8_t* buffer, size_t len, size_t& outLen) {
    outLen = 0;
    if (hComm == INVALID_HANDLE_VALUE)
        return SerialError::PORT_NOT_OPEN;

    DWORD readBytes = 0;
    if (!ReadFile(hComm, buffer, static_cast<DWORD>(len), &readBytes, NULL))
        return SerialError::READ_FAIL;

    if (readBytes == 0)
        return SerialError::TIMEOUT;

    outLen = readBytes;
    return SerialError::NONE;
}
