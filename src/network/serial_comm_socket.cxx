/*******************************************************************************
 * Class: SerialCommSocket
 * Filename: udp_comm_socket.cxx
 * Author: Bill French (wfrench@ucsd.edu)
 * License: Apache 2.0
 *
 * Manage connecting to serial devices
 *
 ******************************************************************************/

#include "serial_comm_socket.h"
#include "common/util.h"
#include "common/logger.h"
#include "common/exception.h"

#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <sys/fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

using namespace std;
using namespace logger;
using namespace network;
    
/******************************************************************************
 *   PUBLIC METHODS
 ******************************************************************************/

/******************************************************************************
 * Method: Constructor
 * Description: Default constructor.
 ******************************************************************************/
SerialCommSocket::SerialCommSocket() {

    m_sDevicePath = "devicePath not initialized!";
    m_baud = B9600;
    m_parity = PARITY_NONE;
    m_dataBits = DATABITS_8;
    m_stopBits = STOPBITS_1;
    m_flowControl = FLOW_CONTROL_NONE;

}


/******************************************************************************
 * Method: Copy Constructor
 * Description: Copy constructor.
 ******************************************************************************/
SerialCommSocket::SerialCommSocket(const SerialCommSocket &rhs) {
}


/******************************************************************************
 * Method: Destructor
 * Description: destructor.
 ******************************************************************************/
SerialCommSocket::~SerialCommSocket() {
}

/******************************************************************************
 * Method: copy
 * Description: return a new object deep copied.
 ******************************************************************************/
CommBase * SerialCommSocket::copy() {
	return new SerialCommSocket(*this);
}

/******************************************************************************
 * Method: Equality operator
 * Description: overloading!
 ******************************************************************************/
bool SerialCommSocket::operator==(SerialCommSocket &rhs) {
		return compare(&rhs);
}

/******************************************************************************
 * Method: compare
 * Description: compare objects
 ******************************************************************************/
bool SerialCommSocket::compare(CommBase *rhs) {
		return false;
}

/******************************************************************************
 * Method: assignment operator
 * Description: overloaded assignment operator.
 ******************************************************************************/
SerialCommSocket & SerialCommSocket::operator=(const SerialCommSocket &rhs) {
	return *this;
}

/******************************************************************************
 * Method: initialize
 * Description: Perform required initialization.
bool SerialCommSocket::initialize() {
	bool bReturnCode = true;

	bIsConfigured = true;
	return bReturnCode;
}
 ******************************************************************************/

/******************************************************************************
 * Method: initializeDevice
 * Description: Perform required initialization.
 ******************************************************************************/
bool SerialCommSocket::initialize() {
    bool bReturnCode = true;

    if (m_pSocketFD) {
        close(m_pSocketFD);
    }
    m_pSocketFD = open(m_sDevicePath.c_str(), O_RDWR);
    if (0 > m_pSocketFD) {
        ostringstream os;
        os << "Failed to open device: " << m_sDevicePath << ": " << strerror(errno);
        string errorString = os.str();
        LOG(ERROR) << errorString;
        throw DeviceOpenFailure(errorString);
        bReturnCode = false;
    }

    return bReturnCode;
}

/******************************************************************************
 * Method: initializeSerialSettings
 * Description: Initialize all serial settings
 ******************************************************************************/
bool SerialCommSocket::initializeSerialSettings() {
    bIsConfigured = true;

    struct termios config, newconfig;

    // now get one to modify
    tcgetattr(m_pSocketFD, &config);

    // Input flags - Turn off input processing
    // convert break to null byte, no CR to NL translation,
    // no NL to CR translation, don't mark parity errors or breaks
    // no input parity check, don't strip high bit off,
    // no XON/XOFF software flow control
    //
    config.c_iflag &= ~(IGNBRK | BRKINT | ICRNL |
                        INLCR | PARMRK | INPCK | ISTRIP | IXON);

    //
    // Output flags - Turn off output processing
    // no CR to NL translation, no NL to CR-NL translation,
    // no NL to CR translation, no column 0 CR suppression,
    // no Ctrl-D suppression, no fill characters, no case mapping,
    // no local output processing
    //
    // config.c_oflag &= ~(OCRNL | ONLCR | ONLRET |
    //                     ONOCR | ONOEOT| OFILL | OLCUC | OPOST);
    config.c_oflag = 0;

    //
    // Set flow control
    //
    if (m_flowControl == FLOW_CONTROL_NONE) {
        config.c_lflag &= ~CRTSCTS;
    }
    else if (m_flowControl == FLOW_CONTROL_HARDWARE) {
        config.c_lflag |= CRTSCTS;
    }
    else if (m_flowControl == FLOW_CONTROL_SOFTWARE) {
        config.c_lflag &= ~CRTSCTS;
        config.c_lflag |= IXON;
    }

    //
    // No line processing:
    // echo off, echo newline off, canonical mode off,
    // extended input processing off, signal chars off
    //
    config.c_lflag &= ~(ECHO | ECHONL | ICANON | IEXTEN | ISIG);

    //
    // Set parity based on given param.
    //
    if (m_parity == PARITY_ODD) {
        config.c_cflag |= (PARENB | PARODD);
    }
    else if (m_parity == PARITY_EVEN) {
        config.c_cflag |= PARENB;
    }
    else {
        config.c_cflag &= ~PARENB;
    }

    //
    // Now set data bits based on given param; clear first
    //
    config.c_cflag &= ~(CSIZE);
    if (m_dataBits == DATABITS_5) {
        config.c_cflag |= CS5;
    }
    else if (m_dataBits == DATABITS_6) {
        config.c_cflag |= CS6;
    }
    else if (m_dataBits == DATABITS_7) {
        config.c_cflag |= CS7;
    }
    else {
        config.c_cflag |= CS8;
    }

    //
    // Set stop bits.  If set to 2, set, otherwise clear.
    //
    if (m_stopBits == STOPBITS_2) {
        config.c_cflag |= CSTOPB;
    }
    else {
        config.c_cflag &= ~CSTOPB;
    }

    //
    // One input byte is enough to return from read()
    // Inter-character timer off
    //
    config.c_cc[VMIN]  = 1;
    config.c_cc[VTIME] = 0;

    //
    // Communication speed (simple version, using the predefined
    // constants)
    //
    if (cfsetispeed(&config, m_baud) < 0 || cfsetospeed(&config, m_baud) < 0) {
        LOG(ERROR) << "set baud failed.";
        bIsConfigured = false;
    }

    //
    // Finally, apply the configuration
    //
    if (tcsetattr(m_pSocketFD, TCSAFLUSH, &config) < 0) {
        LOG(ERROR) << "set serial attribute failed: " << strerror(errno);
        bIsConfigured = false;
    }

    return bIsConfigured;
}

/******************************************************************************
 * Method: isInitialized
 * Description: Has this object been configured?
 ******************************************************************************/
bool SerialCommSocket::isConfigured() {
	return bIsConfigured;
}

bool SerialCommSocket::connected() {
    return (m_pSocketFD > 0);
}

/******************************************************************************
 * Method: write
 * Description: write a number of bytes to the socket connection.  Currently we
 * try to write three times before we fail.  We might want to update this retry
 * so that it keeps retrying if it see progress being made?
 *
 * Parameters:
 *   buffer - the data to write
 *   size - the size of the buffer array
 * Return:
 *   returns the actual number of bytes written.
 * Exceptions:
 *   SocketNotConnected
 *   SocketWriteFailure
 ******************************************************************************/
uint32_t SerialCommSocket::writeData(const char *buffer, const uint32_t size) {
    int bytesWritten = 0;
    int bytesRemaining = size;
    int count;

    if(! connected())
        throw(SocketWriteFailure("not connected"));

    while( bytesRemaining > 0 ) {
        LOG(DEBUG) << "WRITE DEVICE: " << buffer;
        count = write(m_pSocketFD, buffer + bytesWritten, bytesRemaining );
        LOG(DEBUG1) << "bytes written: " << count;
        if(count < 0) {
            m_pSocketFD = 0;
            LOG(ERROR) << strerror(errno) << "(errno: " << errno << ")";
            throw(SocketWriteFailure(strerror(errno)));
        }

        bytesWritten += count;
        bytesRemaining -= count;

        LOG(DEBUG2) << "wrote bytes: " << count << " bytes remaining: " << bytesRemaining;
    }

    return bytesWritten;
}

bool SerialCommSocket::sendBreak(uint32_t  iDuration) {
    bool bReturnCode = true;

    if (0 < tcsendbreak(m_pSocketFD, iDuration)) {
        LOG(ERROR) << "Failed to send break";
        bReturnCode = false;
    }

    return bReturnCode;
}

void SerialCommSocket::setDevicePath(string sDevicePath) {
    LOG(INFO) << "setDevicePath: " << sDevicePath;

    m_sDevicePath = sDevicePath;
}

void SerialCommSocket::setBaud(uint32_t iBaud) {
    LOG(INFO) << "setBaud: " << iBaud;

    if (iBaud == 1200) {
        m_baud = B1200;
    }
    else if (iBaud == 2400) {
        m_baud = B2400;
    }
    else if (iBaud == 4800) {
        m_baud = B4800;
    }
    else if (iBaud == 9600) {
        m_baud = B9600;
    }
    else if (iBaud == 19200) {
        m_baud = B19200;
    }
    else if (iBaud == 38400) {
        m_baud = B38400;
    }
    else if (iBaud == 57600) {
        m_baud = B57600;
    }
    else if (iBaud == 57600) {
        m_baud = B57600;
    }
    else if (iBaud == 115200) {
        m_baud = B115200;
    }
}

void SerialCommSocket::setFlowControl(uint16_t iFlowControl) {
    LOG(INFO) << "setFlowControl: " << iFlowControl;

    m_flowControl = iFlowControl;
}

void SerialCommSocket::setStopBits(uint16_t iStopBits) {
    LOG(INFO) << "setStopBits: " << iStopBits;

    m_stopBits = iStopBits;
}

void SerialCommSocket::setDataBits(uint16_t iDataBits) {
    LOG(INFO) << "setDataBits: " << iDataBits;

    m_stopBits = iDataBits;
}

void SerialCommSocket::setParity(uint16_t iParity) {
    LOG(INFO) << "setParity: " << iParity;

    m_stopBits = iParity;
}
