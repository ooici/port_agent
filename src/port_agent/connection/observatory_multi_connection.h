/*******************************************************************************
 * Class: ObservatoryMultiConnection
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
 * ObservatoryMultiConnection connection;
 *
 * connection.setDataPort(4001);
 * connection.setCommandPort(4000);
 *
 * // Is the data port configured
 * connection.dataConfigured();
 *
 * // Is the command port configured
 * connection.commandConfigured();
 *
 * // Sets up listeners if they are configured.  Subsiquent calls to this
 * // method will only initialize those connections that are uninitialized.
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

#ifndef __OBSERVATORY_MULTI_CONNECTION_H_
#define __OBSERVATORY_MULTI_CONNECTION_H_

#include <list>
#include "port_agent/connection/connection.h"
#include "network/tcp_comm_listener.h"

using namespace std;
using namespace network;

namespace port_agent {
    // Forward reference in order to define the typedefs that follow this.
    class ObservatoryDataSockets;

    // DHE NEW: a list of pointers to TCPCommListener objects; The fact that
    // it's a list be abstracted, so that we can change it to a map for faster
    // lookup in the future.
    //typedef TCPCommListener* ObservatoryDataSocket_T;
    //typedef list<ObservatoryDataSocket_T> ObservatoryDataSockets_T;
    typedef list<TCPCommListener*> ObservatoryDataSockets_T;

    // A singleton class that contains sockets
    class ObservatoryDataSockets {
        public:
            static  ObservatoryDataSockets* instance();
            void    logSockets();
            bool    addSocket(TCPCommListener*);
            TCPCommListener* getFirstSocket();
            TCPCommListener* getNextSocket();

        private:
            // CTOR
            ObservatoryDataSockets();

            static ObservatoryDataSockets* m_pInstance;
            ObservatoryDataSockets_T        m_observatoryDataSockets;
            ObservatoryDataSockets_T::iterator m_socketIt;
    };

    class ObservatoryMultiConnection : public Connection {
        /********************
         *      METHODS     *
         ********************/
        
        public:
            ///////////////////////
            // Public Methods
            ObservatoryMultiConnection();
            ObservatoryMultiConnection(const ObservatoryMultiConnection &rhs);
            virtual ~ObservatoryMultiConnection();
            
            void copy(const ObservatoryMultiConnection &copy);
            
            /* Operators */
            ObservatoryMultiConnection & operator=(const ObservatoryMultiConnection &rhs);

            /* Accessors */
            
            // DHE: this needs to iterate.
            //CommBase *dataConnectionObject() { return &m_oDataSocket; }
            CommBase *dataConnectionObject() { return (CommBase*) NULL; }
            CommBase *commandConnectionObject() { return &m_oCommandSocket; }
            
            PortAgentConnectionType connectionType() { return PACONN_OBSERVATORY_MULTI; }
            
            // Custom configurations for the observatory connection
            void setDataPort(uint16_t port);
            void setCommandPort(uint16_t port);

            void addListener(uint16_t port);
            
            /* Query Methods */
            
            // Do we have complete configuration information for each
            // socket connection?
            bool dataConfigured();
            bool commandConfigured();
            
            // Has the connection been initialized (is it listening?)
            bool dataInitialized() { return false; }
            bool isDataInitialized();
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
            // DHE: should have a multiSocketObject here that abstracts the
            // type of data structure holding the sockets.
            //TCPCommListener m_oDataSocket;
            ObservatoryDataSockets* m_poDataSockets;
            TCPCommListener m_oCommandSocket;
            
    };
}

#endif //__OBSERVATORY_MULTI_CONNECTION_H_
