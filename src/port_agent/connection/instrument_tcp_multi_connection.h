/*******************************************************************************
 * Class: InstrumentTCPMultiConnection
 * Filename: observatory_connection.h
 * Author: Bill French (wfrench@ucsd.edu)
 * License: Apache 2.0
 *
 * Manages the socket connections between the instrument the port agent.
 * This interface consists of a TCP listener on the data port and command port
 * setup in non-blocking mode.
 *
 * Usage:
 *
 * InstrumentTCPMultiConnection connection;
 *
 * connection.setDataPortTx(4001);
 * connection.setDataPortRx(4002);
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
 * // Are the data ports connected
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

#ifndef __INSTRUMENT_TCP_MULTI_CONNECTION_H_
#define __INSTRUMENT_TCP_MULTI_CONNECTION_H_

#include "port_agent/connection/connection.h"
#include "network/tcp_comm_socket.h"

using namespace std;
using namespace network;

namespace port_agent {
    class InstrumentTCPMultiConnection : public Connection {
        /********************
         *      METHODS     *
         ********************/
        
        public:
            ///////////////////////
            // Public Methods
            InstrumentTCPMultiConnection();
            InstrumentTCPMultiConnection(const InstrumentTCPMultiConnection &rhs);
            virtual ~InstrumentTCPMultiConnection();
            
            void initialize();
            void copy(const InstrumentTCPMultiConnection &copy);
            
            /* Operators */
            InstrumentTCPMultiConnection & operator=(const InstrumentTCPMultiConnection &rhs);

            /* Accessors */
            
            // DHE: TEMPTEMP: for now just have this return the Rx socket until I figure out
            // it something else is needed.
            CommBase *dataConnectionObject() { return &m_oDataRxSocket; }
            CommBase *commandConnectionObject() { return NULL; }
            
            PortAgentConnectionType connectionType() { return PACONN_INSTRUMENT_TCP_MULTI; }
            
            // Custom configurations for the observatory connection
            void setDataTxPort(uint16_t port);
            void setDataRxPort(uint16_t port);
            void setDataHost(const string &host);
            
            const string & dataHost() { return m_oDataRxSocket.hostname(); }
            uint16_t dataTxPort() { return m_oDataTxSocket.port(); }
            uint16_t dataRxPort() { return m_oDataRxSocket.port(); }
            bool connected() { return m_oDataTxSocket.connected() && m_oDataRxSocket.connected(); }
            bool disconnect() { return m_oDataTxSocket.disconnect() && m_oDataRxSocket.disconnect(); }
            
            /* Query Methods */
            
            // Do we have complete configuration information for each
            // socket connection?
            bool dataConfigured();
            bool commandConfigured();
            
            // Have the connections been initialized (is the RX listening?)
            bool dataInitialized();
            bool commandInitialized();
            
            // Have connections been made?
            bool dataConnected();
            bool commandConnected();
            
            /* Commands */
            
            // Initialize sockets
            void initializeDataSocket();
            void initializeCommandSocket();
        
        protected:

        private:
        
        /********************
         *      MEMBERS     *
         ********************/
        
        protected:
            
        private:
            TCPCommSocket m_oDataTxSocket;
            TCPCommSocket m_oDataRxSocket;
            
    };
}

#endif //__INSTRUMENT_TCP_CONNECTION_H_
