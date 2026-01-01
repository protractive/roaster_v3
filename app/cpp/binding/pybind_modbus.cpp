#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "../serial/serial_port.h"
#include "../modbus/modbus_rtu.h"

namespace py = pybind11;

class ModbusClientWrapper
{
public:
    ModbusClientWrapper(const std::string &portName, uint32_t baudRate, uint8_t dataBits, uint8_t stopBits, char parity, uint32_t timeoutMs, uint8_t slaveId)
        : slaveId_(slaveId), serialPort_(), modbusRtu_(serialPort_)
    {
        SerialConfig cfg;
        cfg.port = portName;
        cfg.baudRate = baudRate;
        cfg.dataBits = dataBits;
        cfg.stopBits = stopBits;
        cfg.parity = parity;
        cfg.timeoutMs = timeoutMs;

        auto err = serialPort_.open(cfg);
        if (err != SerialError::NONE)
            throw std::runtime_error("Failed to open serial port");
    }

    std::vector<uint16_t> read_holding_register(uint16_t startAddress, uint16_t quantity)
    {
        std::vector<uint16_t> values;
        auto err = modbusRtu_.readHoldingRegisters(slaveId_, startAddress, quantity, values);
        if (err != ModbusError::NONE)
            throw std::runtime_error("Modbus read holding registers error");
        return values;
    }
    std::vector<uint16_t> read_input_register(uint16_t startAddress, uint16_t quantity)
    {
        std::vector<uint16_t> values;
        auto err = modbusRtu_.readInputRegisters(slaveId_, startAddress, quantity, values);
        if (err != ModbusError::NONE)
            throw std::runtime_error("Modbus read input registers error");
        return values;
    }

    void close()
    {
        serialPort_.close();
    }

private:
    uint8_t slaveId_;
    SerialPort serialPort_;
    ModbusRTU modbusRtu_;
}