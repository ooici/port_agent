/*******************************************************************************
 * Class: InstrumentRSNConnection
 * Filename: instrument_rsn_connection.h
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

#ifndef __INSTRUMENT_RSN_CONNECTION_H_
#define __INSTRUMENT_RSN_CONNECTION_H_

#include "port_agent/connection/connection.h"
#include "network/tcp_comm_socket.h"

using namespace std;
using namespace network;

namespace port_agent {
    class InstrumentRSNConnection : public Connection {
        /********************
         *      METHODS     *
         ********************/
        
        public:
            ///////////////////////
            // Public Methods
            InstrumentRSNConnection();
            InstrumentRSNConnection(const InstrumentRSNConnection &rhs);
            virtual ~InstrumentRSNConnection();
            
            void initialize();
            void copy(const InstrumentRSNConnection &copy);
            
            /* Operators */
            InstrumentRSNConnection & operator=(const InstrumentRSNConnection &rhs);

            /* Accessors */

			CommBase *dataConnectionObject() { return &m_oDataSocket; }
			CommBase *commandConnectionObject() { return &m_oCommandSocket; }

            /* Accessors */
            PortAgentConnectionType connectionType() { return PACONN_INSTRUMENT_RSN; }
            
            // Custom configurations for the rsn connection
			void setDataPort(uint16_t port);
			void setDataHost(const string &host);

			void setCommandPort(uint16_t port);
			void setCommandHost(const string &host);

			const string & dataHost() { return m_oDataSocket.hostname(); }
			uint16_t dataPort() { return m_oDataSocket.port(); }

			const string & commandHost() { return m_oCommandSocket.hostname(); }
			uint16_t commandPort() { return m_oCommandSocket.port(); }

			bool connected() { return m_oDataSocket.connected() && m_oCommandSocket.connected(); }
			bool disconnect() { return m_oDataSocket.disconnect() && m_oCommandSocket.disconnect(); }

			/* Query Methods */

			// Do we have complete configuration information for each
			// socket connection?
			bool dataConfigured();
			bool commandConfigured();

			// Has the connection been initialized (is it listening?)
			bool dataInitialized();
			bool commandInitialized();

			// Has a connection been made?
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
            TCPCommSocket m_oDataSocket;
            TCPCommSocket m_oCommandSocket;
            
    };
}

#endif //__INSTRUMENT_RSN_CONNECTION_H_
