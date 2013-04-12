/*******************************************************************************
 * Class: Packet
 * Filename: packet.h
 * Author: Bill French (wfrench@ucsd.edu)
 * License: Apache 2.0
 *
 * Base class for an instrument or observatory connection.  It is responsible
 * for setting up the socket communications for the data port as well as the
 * command port if required.
 *    
 ******************************************************************************/

#ifndef __CONNECTION_H_
#define __CONNECTION_H_

#include "network/comm_base.h"

using namespace std;
using namespace network;

namespace port_agent {
    typedef enum PortAgentConnectionType {
        PACONN_UNKNOWN              = 0x00,
        PACONN_OBSERVATORY_STANDARD = 0x01,
        PACONN_OBSERVATORY_MULTI    = 0x02,
        PACONN_INSTRUMENT_TCP       = 0x03,
        PACONN_INSTRUMENT_BOTPT     = 0x04,
        PACONN_INSTRUMENT_SERIAL    = 0x05
    } PortAgentConnectionType;
    
    class Connection {
        /********************
         *      METHODS     *
         ********************/
        
        public:
            ///////////////////////
            // Public Methods
            Connection();
            Connection(const Connection &rhs);
            virtual ~Connection();
            
            /* Operators */
            virtual Connection & operator=(const Connection &rhs);

            /* Accessors */
            
            virtual CommBase *dataConnectionObject() = 0;
            virtual CommBase *commandConnectionObject() = 0;
            
            virtual PortAgentConnectionType connectionType() = 0;
            
            /* Query Methods */
            
            // Do we have complete configuration information for each
            // socket connection?
            virtual bool dataConfigured() = 0;
            virtual bool commandConfigured() = 0;
            
            // Has the connection been initialized (is it listening?)
            virtual bool dataInitialized() = 0;
            virtual bool commandInitialized() = 0;
            
            // Has a connection been made?
            virtual bool dataConnected() = 0;
            virtual bool commandConnected() = 0;
            
            /* Commands */
            
            // Initialize all uninitialized sockets
            virtual void initialize();
            
            // Every connection has a data socket
            virtual void initializeDataSocket() = 0;
            
            // Not every connection type has a command socket.
            virtual void initializeCommandSocket() {};

            // Send break condition for duration (milliseconds)
            virtual bool sendBreak(uint32_t duration) { return false; }
        
        protected:

        private:
        
        /********************
         *      MEMBERS     *
         ********************/
        
        protected:
        
        private:
            
    };
}

#endif //__CONNECTION_H_
