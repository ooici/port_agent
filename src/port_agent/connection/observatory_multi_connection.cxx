/*******************************************************************************
 * Class: ObservatoryMultiConnection
 * Filename: observatory_botpt_connection.cxx
 * Author: Bill French (wfrench@ucsd.edu)
 * License: Apache 2.0
 *
 * Manages the socket connections between the observatory and the port agent.
 * This interface consists of a TCP listener on the data port and command port
 * setup in non-blocking mode.
 *
 * Usage:
 *
 * ObservatoryMultiConnection connection;
 *
 * connection.setDataPort(4001);
 * connection.setCommandPort(4000);
 *
 * // Sets up listeners if they are configured
 * connection.initialize();
 *
 * // Is the data port initialized (listening)
 * connection.dataInitialized();
 *
 * // Is the command port initialized (listening)
 * connection.commandInitialized();
 *
 * // Is the data port connected
 * connection.dataConnected();
 *
 * // Is the command port connected
 * connection.commandConnected();
 *
 * // Get a pointer tcp data listener object
 * TCPCommListener *data = connection.dataConnectionObject();
 *    
 * // Get a pointer tcp command listener object
 * TCPCommListener *command = connection.commandConnectionObject();
 *    
 ******************************************************************************/

#include "observatory_multi_connection.h"
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
ObservatoryMultiConnection::ObservatoryMultiConnection() : Connection() {
}

/******************************************************************************
 * Method: Copy Constructor
 * Description: Copy constructor ensuring we do a deep copy of the packet data.
 *
 * Parameters:
 *   copy - rhs object to copy
 ******************************************************************************/
ObservatoryMultiConnection::ObservatoryMultiConnection(const ObservatoryMultiConnection& rhs) {
    copy(rhs);
}

/******************************************************************************
 * Method: Destructor
 * Description: free up our dynamically created packet data.
 ******************************************************************************/
ObservatoryMultiConnection::~ObservatoryMultiConnection() {
}

/******************************************************************************
 * Method: Assignemnt operator
 * Description: Deep copy
 *
 * Parameters:
 *   copy - rhs object to copy
 ******************************************************************************/
ObservatoryMultiConnection & ObservatoryMultiConnection::operator=(const ObservatoryMultiConnection &rhs) {
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
void ObservatoryMultiConnection::copy(const ObservatoryMultiConnection &copy) {
    //m_oDataSocket = copy.m_oDataSocket;
    m_oCommandSocket = copy.m_oCommandSocket;
}

/******************************************************************************
 * Method: addDataPort
 * Description: Set the data socket listener port.  First we want to see if the
 * port is bound and is listening on a port other than the target port we are
 * setting up.  If it is different, then we need to disconnect so it can be
 * bound to a new port.
void ObservatoryMultiConnection::setDataPort(uint16_t port) {

    // DHE: this needs multiple sockets.
    m_oDataSocket.setPort(port);
    
    if(m_oDataSocket.listening() && m_oDataSocket.port() != m_oDataSocket.getListenPort()) {
	m_oDataSocket.initialize();
    }
}
 ******************************************************************************/

/******************************************************************************
 * Method: addListener
 * Description: Add a listener for the given port.
 ******************************************************************************/
void ObservatoryMultiConnection::addListener(uint16_t port) {

    // DHE: this needs multiple sockets.
    TCPCommListener *listener = new TCPCommListener();
    listener->setPort(port);
    listener->initialize();
    ObservatoryDataSockets::instance()->addSocket(listener);
    //m_poDataSockets.setPort(port);

    /***
    if(m_oDataSocket.listening() && m_oDataSocket.port() != m_oDataSocket.getListenPort()) {
    m_oDataSocket.initialize();
    }
    ***/
}

/******************************************************************************
 * Method: setCommandPort
 * Description: Set the command socket listener port
 ******************************************************************************/
void ObservatoryMultiConnection::setCommandPort(uint16_t port) {
    m_oCommandSocket.setPort(port);
}

/******************************************************************************
 * Method: dataConfigured
 * Description: Do we have enough configuration information to initialize the
 * data socket?  In order to support this for the multiconnection, we need to
 * iterate through the listeners checking to see if all are configured.  If
 * any one isn't, we return false.
 *
 * Return: 
 *   True if we have enough configuration information
 ******************************************************************************/
bool ObservatoryMultiConnection::dataConfigured() {
    TCPCommListener* pListener = 0;
    bool bConfigured = true;

    pListener = ObservatoryDataSockets::instance()->getFirstSocket();
    while (pListener) {
        if (!pListener->isConfigured()) {
            bConfigured = false;
            break;
        }
        pListener = ObservatoryDataSockets::instance()->getNextSocket();
    }

    return bConfigured;
}

/******************************************************************************
 * Method: commandConfigured
 * Description: Do we have enough configuration information to initialize the
 * command socket?
 *
 * Return: 
 *   True if we have enough configuration information
 ******************************************************************************/
bool ObservatoryMultiConnection::commandConfigured() {
    return m_oCommandSocket.port() && m_oCommandSocket.isConfigured();
}

/******************************************************************************
 * Method: dataInitialized
 * Description: Has the data socket been initialized yet? Is it listening?
 * In order to support this for the multiconnection, we need to
 * iterate through the listeners checking to see if all are listening.  If
 * any one isn't, we return false.
 * Return:
 *   True if the socket has been configured and is bound to a port listening
 ******************************************************************************/
bool ObservatoryMultiConnection::isDataInitialized() {
    TCPCommListener* pListener = 0;
    bool bListening = true;

    pListener = ObservatoryDataSockets::instance()->getFirstSocket();
    while (pListener) {
        if (!pListener->listening()) {
            bListening = false;
            break;
        }
        pListener = ObservatoryDataSockets::instance()->getNextSocket();
    }

    return bListening;
}

/******************************************************************************
 * Method: commandInitialized
 * Description: Has the command socket been initialized yet? Is it listening?
 *
 * Return:
 *   True if the socket has been configured and is bound to a port listening
 ******************************************************************************/
bool ObservatoryMultiConnection::commandInitialized() {
    return m_oCommandSocket.listening();
}

/******************************************************************************
 * Method: dataConnected
 * Description: Is a client connected to the data socket connection
 * In order to support this for the multiconnection, we need to
 * iterate through the listeners checking to see if all are connected.  If
 * any one isn't, we return false.
 * Return:
 *   True if the data socket is connected
 ******************************************************************************/
bool ObservatoryMultiConnection::dataConnected() {
    TCPCommListener* pListener = 0;
    bool bConnected = true;

    pListener = ObservatoryDataSockets::instance()->getFirstSocket();
    while (pListener) {
        if (!pListener->listening()) {
            bConnected = false;
            break;
        }
        pListener = ObservatoryDataSockets::instance()->getNextSocket();
    }

    return bConnected;
}

/******************************************************************************
 * Method: commandConnected
 * Description: Is a client connected to the command connection?
 *
 * Return:
 *   True if the command socket is connected
 ******************************************************************************/
bool ObservatoryMultiConnection::commandConnected() {
    return m_oCommandSocket.connected();
}

/******************************************************************************
 * Method: initializeDataSocket
 * Description: Initialize the data socket
 ******************************************************************************/
void ObservatoryMultiConnection::initializeDataSocket() {
    TCPCommListener* pListener = 0;

    pListener = ObservatoryDataSockets::instance()->getFirstSocket();
    while (pListener) {
        pListener->initialize();
        pListener = ObservatoryDataSockets::instance()->getNextSocket();
    }
}

/******************************************************************************
 * Method: initializeCommandSocket
 * Description: Initialize the command socket
 ******************************************************************************/
void ObservatoryMultiConnection::initializeCommandSocket() {
    m_oCommandSocket.initialize();
}

ObservatoryDataSockets* ObservatoryDataSockets::m_pInstance = 0;

ObservatoryDataSockets::ObservatoryDataSockets() {

}

/******************************************************************************
 * Method: instance()
 * Description: Return a pointer to the singleton instance of the
 * ObservatoryDataSockets container.  If the singleton does not yet exist, create
 * it.
 * Return: return a pointer to the instance.
 ******************************************************************************/
ObservatoryDataSockets* ObservatoryDataSockets::instance() {

    if (0 == m_pInstance) {
       m_pInstance = new ObservatoryDataSockets();
    }

    return m_pInstance;
}

/******************************************************************************
 * Method: logPorts()
 * Description: Log the ports
 * Return: void
 ******************************************************************************/
void ObservatoryDataSockets::logSockets() {

    list<TCPCommListener*>::iterator i = m_observatoryDataSockets.begin();
    int j = 0;
    for (i = m_observatoryDataSockets.begin(); i != m_observatoryDataSockets.end(); i++) {
        LOG(DEBUG) << "Data port: " << j << ", " << (*i)->clientFD();
        j++;
    }
}

/******************************************************************************
 * Method: addSocket(TCPCommListener* pSocket)
 * Description: Add the given socket to the container of listener objects.
 * Return: return true if success, false if not.
 ******************************************************************************/
bool ObservatoryDataSockets::addSocket(TCPCommListener* pSocket) {
    bool bRetVal = true;

    LOG(DEBUG) << "ObservatoryDataSockets::addSocket: Adding socket: " << pSocket->serverFD();

    // First remove any existing element with the same port value
    m_observatoryDataSockets.remove(pSocket);
    m_observatoryDataSockets.push_back(pSocket);

    return bRetVal;
}

/******************************************************************************
 * Method: getFirstPort
 * Description: Get the first port from the container of socket objects.
 * Return: value of first port if there is one, otherwise 0
 ******************************************************************************/
TCPCommListener* ObservatoryDataSockets::getFirstSocket()
{

   // get the first cam agent
   m_socketIt = m_observatoryDataSockets.begin();

   // if the first is the end, there are no ports in the list
   if (m_socketIt == m_observatoryDataSockets.end()) {
      return 0;
   }
   else {
      return *m_socketIt;
   }
}

/******************************************************************************
 * Method: getNextPort
 * Description: Get the next port from the container of socket objects.
 * Return: value of next port if there is one, otherwise 0
 ******************************************************************************/
TCPCommListener* ObservatoryDataSockets::getNextSocket()
{

   m_socketIt++;

   // if the next is the end, there are no more ports in the list
   if (m_socketIt == m_observatoryDataSockets.end()) {
      return 0;
   }
   else {
      return *m_socketIt;
   }
}





