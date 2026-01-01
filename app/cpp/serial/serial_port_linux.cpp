#include "serial_port.h"

#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>
#include <cstring>

static speed_t toBaud(int baud)
{
    switch (baud)
    {
    case 9600:
        return B9600;
    case 19200:
        return B19200;
    case 38400:
        return B38400;
    case 57600:
        return B57600;
    case 115200:
        return B115200;
    default:
        return B9600; // Default to 9600 if unsupported baud rate
    }
}

SerialPort::SerialPort() : fd_(-1), timeoutMs_(1000) {}
SerialPort::~SerialPort() { close(); }

bool SerialPort::isOpen() const
{
    return fd_ != -1;
}

SerialError SerialPort::open(const SerialConfig &config)
{
    timeoutMs_ = config.timeoutMs;

    // Open the serial port
    fd_ = ::open(config.port.c_str(), O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (fd_ == -1)
        return SerialError::PORT_OPEN_FAIL;

    // Configure the serial port
    termios tty{};
    if (tcgetattr(fd_, &tty) != 0)
    {
        close();
        return SerialError::CONFIG_FAIL;
    }
    cfmakeraw(&tty);

    // baud rate
    speed_t speed = toBaud(config.baudRate);
    cfsetispeed(&tty, speed);
    cfsetospeed(&tty, speed);

    // data bits
    tty.c_cflag &= ~CSIZE;
    tty.c_cflag |= (config.dataBits == 7) ? CS7 : CS8;

    // parity
    if (config.parity == 'E')
    {
        tty.c_cflag |= PARENB;
        tty.c_cflag &= ~PARODD;
    }
    else if (config.parity == 'O')
    {
        tty.c_cflag |= PARENB;
        tty.c_cflag |= PARODD;
    }
    else
    {
        tty.c_cflag &= ~PARENB;
    }

    // stop bits
    if (config.stopBits == 2)
        tty.c_cflag |= CSTOPB;
    else
        tty.c_cflag &= ~CSTOPB;

    tty.c_cflag |= (CLOCAL | CREAD);

    // No flow control
    tty.c_iflag &= ~(IXON | IXOFF | IXANY);
    tty.c_cflag &= ~CRTSCTS;

    tty.c_cc[VMIN] = 0;
    tty.c_cc[VTIME] = 0;

    if (tcsetattr(fd_, TCSANOW, &tty) != 0)
    {
        close();
        return SerialError::CONFIG_FAIL;
    }

    return SerialError::NONE;
}

void SerialPort::close()
{
    if (isOpen())
    {
        ::close(fd_);
        fd_ = -1;
    }
}

SerialError SerialPort::write(const uint8_t *data, size_t len)
{
    if (!isOpen())
        return SerialError::PORT_NOT_OPEN;

    size_t sent = 0;
    while (sent < len)
    {
        ssize_t w = ::write(fd_, data + sent, len - sent);
        if (w < 0)
            return SerialError::WRITE_FAIL;
        sent += w;
    }
    return SerialError::NONE;
}

SerialError SerialPort::read(uint8_t *buffer, size_t len, size_t &outLen)
{
    outLen = 0;
    if (!isOpen())
        return SerialError::PORT_NOT_OPEN;

    pollfd pfd{};
    pfd.fd = fd_;
    pfd.events = POLLIN;

    int ret = poll(&pfd, 1, timeoutMs_);
    if (ret == 0)
        return SerialError::TIMEOUT;
    if (ret < 0)
        return SerialError::READ_FAIL;

    ssize_t r = ::read(fd_, buffer, len);
    if (r < 0)
        return SerialError::READ_FAIL;

    outLen = static_cast<size_t>(r);
    return SerialError::NONE;
}
