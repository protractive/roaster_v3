#pragma once
#include <cstdint>
#include <vector>
#include "../../serial/serial_port.h"

enum class ModbusError
{
    NONE = 0,
    TIMEOUT,
    CRC_ERROR,
    EXCEPTION,
    INVALID_RESPONSE,
    SERIAL_ERROR,
};

class ModbusRTU
{
public:
    explicit ModbusRTU(SerialPort &serialPort);

    // Read holding registers
    ModbusError readHoldingRegisters(uint8_t slaveId, uint16_t startAddress, uint16_t quantity, std::vector<uint16_t> &outValue);

    // Read input registers
    ModbusError readInputRegisters(uint8_t slaveId, uint16_t startAddress, uint16_t quantity, std::vector<uint16_t> &outValue);

private:
    SerialPort &serialPort_;

    ModbusError readRegisterInternal(uint8_t functionCode, uint8_t slaveId, uint16_t startAddress, uint16_t quantity, std::vector<uint16_t> &outValue);
};