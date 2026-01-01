#include "modbus_rtu.h"
#include "common/crc16.h"

static void buildReadRequest(uint8_t slave, uint8_t func, uint16_t addr, uint16_t qty, uint8_t *req)
{
    req[0] = slave;
    req[1] = func;
    req[2] = (addr >> 8) & 0xFF;
    req[3] = addr & 0xFF;
    req[4] = (qty >> 8) & 0xFF;
    req[5] = qty & 0xFF;
    uint16_t crc = modbus_crc16(req, 6);
    req[6] = crc & 0xFF;
    req[7] = (crc >> 8) & 0xFF;
}

ModbusRTU::ModbusRTU(SerialPort &serialPort) : serialPort_(serialPort) {}

ModbusError ModbusRTU::readHoldingRegisters(uint8_t slaveId, uint16_t startAddress, uint16_t quantity, std::vector<uint16_t> &outValue)
{
    return readRegisterInternal(0x03, slaveId, startAddress, quantity, outValue);
}

ModbusError ModbusRTU::readInputRegisters(uint8_t slaveId, uint16_t startAddress, uint16_t quantity, std::vector<uint16_t> &outValue)
{
    return readRegisterInternal(0x04, slaveId, startAddress, quantity, outValue);
}

ModbusError ModbusRTU::readRegisterInternal(uint8_t functionCode, uint8_t slaveId, uint16_t startAddress, uint16_t quantity, std::vector<uint16_t> &outValue)
{
    outValue.clear();

    // request frame
    uint8_t req[8];
    buildReadRequest(slaveId, functionCode, startAddress, quantity, req);
    if (serialPort_.write(req, sizeof(req)) != SerialError::NONE)
        return ModbusError::SERIAL_ERROR;

    // response frame
    const size_t expectedDataBytes = quantity * 2;
    const size_t maxResponseLen = 5 + expectedDataBytes;
    uint8_t response[0xFF] = {0};

    size_t len = 0;
    auto sErr = serialPort_.read(response, sizeof(response), len);

    if (sErr == SerialError::TIMEOUT)
        return ModbusError::TIMEOUT;
    if (sErr != SerialError::NONE)
        return ModbusError::SERIAL_ERROR;

    if (len < 5)
        return ModbusError::INVALID_RESPONSE;

    // verify CRC
    uint16_t responseCrc = response[len - 2] | (response[len - 1] << 8);
    uint16_t calcCrc = modbus_crc16(response, len - 2);
    if (responseCrc != calcCrc)
        return ModbusError::CRC_ERROR;

    // verify slave ID and function code
    if (response[0] != slaveId)
        return ModbusError::INVALID_RESPONSE;

    if (response[1] == 0x80)
        return ModbusError::EXCEPTION;

    if (response[1] != functionCode)
        return ModbusError::INVALID_RESPONSE;

    uint8_t byteCount = response[2];
    if (byteCount != expectedDataBytes)
        return ModbusError::INVALID_RESPONSE;

    // data extraction
    outValue.reserve(quantity);
    for (size_t i = 0; i < quantity; ++i) {
        uint16_t v = response[3 + i * 2] << 8 | response[4 + i * 2];
        outValue.push_back(v);
    }

    return ModbusError::NONE;
}

