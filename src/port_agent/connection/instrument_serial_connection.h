/*******************************************************************************
 * Class: InstrumentSerialConnection
 * Filename: observatory_connection.h
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
 * SerialCommListener *data = connection.dataConnectionObject();
 *    
 * // Always returns null for this connection type
 * SerialCommListener *command = connection.commandConnectionObject();
 *    
 ******************************************************************************/

#ifndef __INSTRUMENT_SERIAL_CONNECTION_H_
#define __INSTRUMENT_SERIAL_CONNECTION_H_

#include "port_agent/connection/connection.h"
#include "network/serial_comm_socket.h"

using namespace std;
using namespace network;

namespace port_agent {
    class InstrumentSerialConnection : public Connection {
        /********************
         *      METHODS     *
         ********************/
        
        public:
            ///////////////////////
            // Public Methods
            InstrumentSerialConnection();
            InstrumentSerialConnection(const InstrumentSerialConnection &rhs);
            virtual ~InstrumentSerialConnection();
            
            void initialize();
            void copy(const InstrumentSerialConnection &copy);
            
            /* Operators */
            InstrumentSerialConnection & operator=(const InstrumentSerialConnection &rhs);

            /* Accessors */
            
            CommBase *dataConnectionObject() { return &m_oDataSocket; }
            CommBase *commandConnectionObject() { return NULL; }
            
            PortAgentConnectionType connectionType() { return PACONN_INSTRUMENT_SERIAL; }
            
            // Custom configurations for the observatory connection
            void setDevicePath(const string &devicePath);
            void setBaud(const uint32_t &iBaud);
            void setFlowControl(const uint16_t &iFlowControl);
            void setStopBits(const uint16_t &iStopBits);
            void setDataBits(const uint16_t &iDataBits);
            void setParity(const uint16_t &iParity);
            bool initializeSerialSettings();
            
            const string & devicePath() { return m_oDataSocket.devicePath(); }
            bool connected() { return m_oDataSocket.connected(); }
            bool disconnect() { return m_oDataSocket.disconnect(); }
            
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

            // Send break condition for duration (milliseconds)
            virtual bool sendBreak(const uint32_t duration);

        
        protected:

        private:
        
        /********************
         *      MEMBERS     *
         ********************/
        
        protected:
            
        private:
            SerialCommSocket m_oDataSocket;
            
    };
}

#endif //__INSTRUMENT_SERIAL_CONNECTION_H_
