/*******************************************************************************
 * Class: InstrumentSerialConnection
 * Filename: instrument_tcp_connection.cxx
 * Author: Bill French (wfrench@ucsd.edu)
 * License: Apache 2.0
 *
 * Manages the socket connection between the observatory and the port agent.
 * This interface consists of a TCP listener on the data port and command port
 * setup in non-blocking mode.
 *
 * Usage:
 *
 * InstrumentSerialConnection connection;
 *
 * connection.setDataPort(4001);
 *
 * // Is the data port configured
 * connection.dataConfigured();
 *
 * // This is a noop for this method.  There is nothing to initialize
 * connection.initialize();
 *
 * // Always true for this connection type.
 * connection.dataInitialized();
 *
 * // Is the data port connected
 * connection.dataConnected();
 *
 * // Always false for this connection type
 * connection.commandConnected();
 *
 * // Get a pointer tcp data listener object
 * TCPCommListener *data = connection.dataConnectionObject();
 *    
 * // Always returns null for this connection type
 * TCPCommListener *command = connection.commandConnectionObject();
 *    
 ******************************************************************************/

#include "instrument_serial_connection.h"
#include "common/util.h"
#include "common/logger.h"
#include "common/exception.h"
#include "network/tcp_comm_listener.h"

using namespace std;
using namespace logger;
using namespace network;
using namespace port_agent;
    
/******************************************************************************
 *   PUBLIC METHODS
 ******************************************************************************/
/******************************************************************************
 * Method: Constructor
 * Description: Default constructor.  Is likely never called, but wanted to
 *              define it explicitly.
 ******************************************************************************/
InstrumentSerialConnection::InstrumentSerialConnection() : Connection() {
}

/******************************************************************************
 * Method: Copy Constructor
 * Description: Copy constructor ensuring we do a deep copy of the packet data.
 *
 * Parameters:
 *   copy - rhs object to copy
 ******************************************************************************/
InstrumentSerialConnection::InstrumentSerialConnection(const InstrumentSerialConnection& rhs) {
    copy(rhs);
}

/******************************************************************************
 * Method: Destructor
 * Description: free up our dynamically created packet data.
 ******************************************************************************/
InstrumentSerialConnection::~InstrumentSerialConnection() {
}

/******************************************************************************
 * Method: Assignment operator
 * Description: Deep copy
 *
 * Parameters:
 *   copy - rhs object to copy
 ******************************************************************************/
InstrumentSerialConnection & InstrumentSerialConnection::operator=(const InstrumentSerialConnection &rhs) {
    copy(rhs);
    return *this;
}


/******************************************************************************
 * Method: copy
 * Description: Copy data from one packet object into this one.  Ensuring we
 * deep copy when needed.
 *
 * Parameters:
 *   copy - rhs object to copy
 ******************************************************************************/
void InstrumentSerialConnection::copy(const InstrumentSerialConnection &copy) {
    m_oDataSocket = copy.m_oDataSocket;
}

/******************************************************************************
 * Method: setDevicePath
 * Description: Set the device path to the device driver for the serial port.
 ******************************************************************************/
void InstrumentSerialConnection::setDevicePath(const string & devicePath) {
	m_oDataSocket.setDevicePath(devicePath);
}

/******************************************************************************
 * Method: setBaud
 * Description: Set the baud.
 ******************************************************************************/
void InstrumentSerialConnection::setBaud(const uint32_t &iBaud) {
    LOG(INFO) << "setBaud: " << iBaud;
    m_oDataSocket.setBaud(iBaud);
}

/******************************************************************************
 * Method: setFlowControl
 * Description: Set the baud.
 ******************************************************************************/
void InstrumentSerialConnection::setFlowControl(const uint16_t &iFlowControl) {
    LOG(INFO) << "setFlowControl: " << iFlowControl;
    m_oDataSocket.setFlowControl(iFlowControl);
}

/******************************************************************************
 * Method: setStopBits
 * Description: Set the baud.
 ******************************************************************************/
void InstrumentSerialConnection::setStopBits(const uint16_t &iStopBits) {
    LOG(INFO) << "setStopBits: " << iStopBits;
    m_oDataSocket.setStopBits(iStopBits);
}

/******************************************************************************
 * Method: setDataBits
 * Description: Set the baud.
 ******************************************************************************/
void InstrumentSerialConnection::setDataBits(const uint16_t &iDataBits) {
    LOG(INFO) << "setDataBits: " << iDataBits;
    m_oDataSocket.setDataBits(iDataBits);
}

/******************************************************************************
 * Method: setParity
 * Description: Set the baud.
 ******************************************************************************/
void InstrumentSerialConnection::setParity(const uint16_t &iParity) {
    LOG(INFO) << "setParity: " << iParity;
    m_oDataSocket.setParity(iParity);
}

/******************************************************************************
 * Method: dataConfigured
 * Description: Do we have enough configuration information to initialize the
 * data socket?
 *
 * Return: 
 *   True if we have enough configuration information
 ******************************************************************************/
bool InstrumentSerialConnection::dataConfigured() {
    return m_oDataSocket.isConfigured();
}

/******************************************************************************
 * Method: commandConfigured
 * Description: Do we have enough configuration information to initialize the
 * command socket?
 *
 * Return: 
 *   True if we have enough configuration information
 ******************************************************************************/
bool InstrumentSerialConnection::commandConfigured() {
    return false;
}

/******************************************************************************
 * Method: dataInitialized
 * Description: No initialization sequence, so if configure then we are
 * initialiaze
 *
 * Return:
 *   True if configured.
 ******************************************************************************/
bool InstrumentSerialConnection::dataInitialized() {
    return dataConfigured();
}

/******************************************************************************
 * Method: commandInitialized
 * Description: Alwasy false because there is no command interface for this
 * connection type.
 *
 * Return:
 *   False
 ******************************************************************************/
bool InstrumentSerialConnection::commandInitialized() {
    return false;
}

/******************************************************************************
 * Method: dataConnected
 * Description: Is a client connected to the data socket connection
 *
 * Return:
 *   True if the data socket is connected
 ******************************************************************************/
bool InstrumentSerialConnection::dataConnected() {
    return m_oDataSocket.connected();
}

/******************************************************************************
 * Method: commandConnected
 * Description: Alwasy false because there is no command interface for this
 * connection type.
 *
 * Return:
 *   False
 ******************************************************************************/
bool InstrumentSerialConnection::commandConnected() {
    return false;
}

/******************************************************************************
 * Method: initializeDataSocket
 * Description: Initialize the data socket
 ******************************************************************************/
void InstrumentSerialConnection::initializeDataSocket() {
    m_oDataSocket.initialize();
}

/******************************************************************************
 * Method: initializeCommandSocket
 * Description: NOOP
 ******************************************************************************/
void InstrumentSerialConnection::initializeCommandSocket() {
}

/******************************************************************************
 * Method: initializeSerialSettings
 * Description: Initialize serial settings.
 ******************************************************************************/
bool InstrumentSerialConnection::initializeSerialSettings() {
        return m_oDataSocket.initializeSerialSettings();
}

/******************************************************************************
 * Method: initialize
 * Description: Initialize any uninitialized sockets if they are configured.
 ******************************************************************************/
void InstrumentSerialConnection::initialize() {
    if(!dataConfigured())
        LOG(DEBUG) << "Data port not configured. Not initializing";
	
    if(dataConfigured() && ! dataConnected()) {
	LOG(DEBUG) << "initialize data socket";
        initializeDataSocket();
    } 
}

/******************************************************************************
 * Method: sendBreak
 * Description: Send a break condition for the given duration.
 ******************************************************************************/
bool InstrumentSerialConnection::sendBreak(const uint32_t iDuration) {
    bool bReturnCode = true;

     if (!m_oDataSocket.sendBreak(iDuration)) {
        LOG(ERROR) << "Failed to send break.";
        bReturnCode = false;
    }

    return bReturnCode;
}
