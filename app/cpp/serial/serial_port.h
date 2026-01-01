#pragma once

#include <string>
#include <cstdint>
#include <cstddef>

enum class SerialError
{
    NONE = 0,
    PORT_OPEN_FAIL,
    PORT_NOT_OPEN,
    CONFIG_FAIL,
    WRITE_FAIL,
    READ_FAIL,
    TIMEOUT,
};

struct SerialConfig
{
    std::string port;
    int baudRate;
    int dataBits;
    int stopBits;
    char parity;
    int timeoutMs;
};

class SerialPort
{
public:
    SerialPort();
    ~SerialPort();

    SerialError open(const SerialConfig& config);
    void close();
    bool isOpen() const;

    SerialError write(const uint8_t* data, size_t len);
    SerialError read(uint8_t* buffer, size_t len, size_t& outLen);

private:
    int fd_;
    int timeoutMs_;
};