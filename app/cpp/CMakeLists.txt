find_package(pybind11 REQUIRED)

pybind11_add_module(
    modbus_engine
    binding/pybind_modbus.cpp
    serial/serial_port_linux.cpp
    modbus/modbus_rtu.cpp
)