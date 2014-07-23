/*******************************************************************************
 * Class: InstrumentRSNConnection
 * Filename: instrument_rsn_connection.cxx
 * Author: Tapana Gupta
 * License: Apache 2.0
 *
 * Manages the socket connection between an RSN Instrument and the port agent.
 * This interface consists of a TCP listener on the data port and command port
 * setup in non-blocking mode.
 *
 * Usage:
 *
 * InstrumentRSNConnection connection;
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

#include "instrument_tcp_connection.h"
#include "instrument_rsn_connection.h"
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
InstrumentRSNConnection::InstrumentRSNConnection() : Connection() {
}

/******************************************************************************
 * Method: Copy Constructor
 * Description: Copy constructor ensuring we do a deep copy of the packet data.
 *
 * Parameters:
 *   copy - rhs object to copy
 ******************************************************************************/
InstrumentRSNConnection::InstrumentRSNConnection(const InstrumentRSNConnection& rhs) {
    copy(rhs);
}

/******************************************************************************
 * Method: Destructor
 * Description: free up our dynamically created packet data.
 ******************************************************************************/
InstrumentRSNConnection::~InstrumentRSNConnection() {
}

/******************************************************************************
 * Method: Assignment operator
 * Description: Deep copy
 *
 * Parameters:
 *   copy - rhs object to copy
 ******************************************************************************/
InstrumentRSNConnection & InstrumentRSNConnection::operator=(const InstrumentRSNConnection &rhs) {
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
void InstrumentRSNConnection::copy(const InstrumentRSNConnection &copy) {
    m_oDataSocket = copy.m_oDataSocket;
    m_oCommandSocket = copy.m_oCommandSocket;
}

/******************************************************************************
 * Method: setDataPort
 * Description: Set the port.  If we are already connected then we need to
 * disconnect and reconnect to the new port.
 ******************************************************************************/
void InstrumentRSNConnection::setDataPort(uint16_t port) {
    uint16_t oldPort = m_oDataSocket.port();
    m_oDataSocket.setPort(port);

    if(m_oDataSocket.connected() && m_oDataSocket.port() != oldPort) {
		m_oDataSocket.initialize();
	}
}

/******************************************************************************
 * Method: setDataHost
 * Description: Set the host.  If we are already connected then we need to
 * disconnect and reconnect to the new port.
 ******************************************************************************/
void InstrumentRSNConnection::setDataHost(const string & host) {
    string oldhost = m_oDataSocket.hostname();
    m_oDataSocket.setHostname(host);

    if(m_oDataSocket.connected() && m_oDataSocket.hostname() != oldhost) {
    	m_oDataSocket.initialize();
    }
}

/******************************************************************************
 * Method: setCommandPort
 * Description: Set the port.  If we are already connected then we need to
 * disconnect and reconnect to the new port.
 ******************************************************************************/
void InstrumentRSNConnection::setCommandPort(uint16_t port) {
    uint16_t oldPort = m_oCommandSocket.port();
    m_oCommandSocket.setPort(port);

    if(m_oCommandSocket.connected() && m_oCommandSocket.port() != oldPort) {
    	m_oCommandSocket.initialize();
    }
}

/******************************************************************************
 * Method: setCommandHost
 * Description: Set the host.  If we are already connected then we need to
 * disconnect and reconnect to the new port.
 ******************************************************************************/
void InstrumentRSNConnection::setCommandHost(const string & host) {
    string oldhost = m_oCommandSocket.hostname();
    m_oCommandSocket.setHostname(host);

    if(m_oCommandSocket.connected() && m_oCommandSocket.hostname() != oldhost) {
    	m_oCommandSocket.initialize();
    }
}

/******************************************************************************
 * Method: dataConfigured
 * Description: Do we have enough configuration information to initialize the
 * data socket?
 *
 * Return:
 *   True if we have enough configuration information
 ******************************************************************************/
bool InstrumentRSNConnection::dataConfigured() {
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
bool InstrumentRSNConnection::commandConfigured() {
    return m_oCommandSocket.isConfigured();
}

/******************************************************************************
 * Method: dataInitialized
 * Description: No initialization sequence, so if configure then we are
 * initialized
 *
 * Return:
 *   True if configured.
 ******************************************************************************/
bool InstrumentRSNConnection::dataInitialized() {
    return dataConfigured();
}

/******************************************************************************
 * Method: commandInitialized
 * Description: No initialization sequence, so if configure then we are
 * initialized
 *
 * Return:
 *   True if configured.
 ******************************************************************************/
bool InstrumentRSNConnection::commandInitialized() {
    return commandConfigured();
}

/******************************************************************************
 * Method: dataConnected
 * Description: Is a client connected to the data socket connection
 *
 * Return:
 *   True if the data socket is connected
 ******************************************************************************/
bool InstrumentRSNConnection::dataConnected() {
    return m_oDataSocket.connected();
}

/******************************************************************************
 * Method: commandConnected
 * Description: Is a client connected to the command socket connection
 *
 * Return:
 *   True if the command socket is connected
 ******************************************************************************/
bool InstrumentRSNConnection::commandConnected() {
    return m_oCommandSocket.connected();
}

/******************************************************************************
 * Method: initializeDataSocket
 * Description: Initialize the data socket
 ******************************************************************************/
void InstrumentRSNConnection::initializeDataSocket() {
    m_oDataSocket.initialize();
}

/******************************************************************************
 * Method: initializeCommandSocket
 * Description: Initialize the command socket
 ******************************************************************************/
void InstrumentRSNConnection::initializeCommandSocket() {
	m_oCommandSocket.initialize();
}


/******************************************************************************
 * Method: initialize
 * Description: Initialize any uninitialized sockets if they are configured.
 ******************************************************************************/
void InstrumentRSNConnection::initialize() {
    if(!dataConfigured())
        LOG(DEBUG) << "Data port not configured. Not initializing";

    if(!commandConfigured())
        LOG(DEBUG) << "Command port not configured. Not initializing";
	
    if(dataConfigured() && !dataConnected()) {
    	LOG(DEBUG) << "initialize data socket";
        initializeDataSocket();
    } 

    if(commandConfigured() && !commandConnected()) {
    	LOG(DEBUG) << "initialize command socket";
		initializeCommandSocket();
	}
}
