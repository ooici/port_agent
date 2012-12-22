/*******************************************************************************
 * Class: SerialCommSocket
 * Filename: udp_comm_socket.h
 * Author: Bill French (wfrench@ucsd.edu)
 * License: Apache 2.0
 *
 * Manage connecting to serial devices
 *
 ******************************************************************************/

#ifndef __SERIAL_COMM_SOCKET_H_
#define __SERIAL_COMM_SOCKET_H_

#include "common/logger.h"
#include "network/comm_socket.h"

using namespace std;
using namespace logger;
using namespace network;

namespace network {

    const uint16_t FLOW_CONTROL_NONE     = 0;
    const uint16_t FLOW_CONTROL_HARDWARE = 1;
    const uint16_t FLOW_CONTROL_SOFTWARE = 2;
    const uint16_t PARITY_NONE = 0;
    const uint16_t PARITY_ODD = 1;
    const uint16_t PARITY_EVEN = 2;
    const uint16_t DATABITS_5 = 5;
    const uint16_t DATABITS_6 = 6;
    const uint16_t DATABITS_7 = 7;
    const uint16_t DATABITS_8 = 8;
    const uint16_t STOPBITS_1 = 1;
    const uint16_t STOPBITS_2 = 2;


    class SerialCommSocket : public CommSocket {
        /********************
         *      METHODS     *
         ********************/
        
        public:
            ///////////////////////
            // Public Methods
    	    SerialCommSocket();
    	    SerialCommSocket(const SerialCommSocket &rhs);
            virtual ~SerialCommSocket();
            
            virtual bool operator==(SerialCommSocket &rhs);
			CommType type() { return COMM_SERIAL_SOCKET; }

            // Initialize
            bool initializeSerialSettings();
            bool initialize();
			
            // Does this object have a complete configuration?
            bool isConfigured();

            virtual CommBase *copy();
	    
            virtual bool compare(CommBase *rhs);
            virtual bool connectClient() { return false; }

            virtual uint32_t writeData(const char *buffer, uint32_t size);
            bool sendBreak(uint32_t iDuration);
            void setDevicePath(string sDevicePath);
            const string &devicePath() { return m_sDevicePath; }

            void setBaud(uint32_t iBaud);
            void setFlowControl(uint16_t iFlowControl);
            void setStopBits(uint16_t iStopBits);
            void setDataBits(uint16_t iDataBits);
            void setParity(uint16_t iParity);
            
            /* Operators */
            virtual SerialCommSocket & operator=(const SerialCommSocket &rhs);

            /* Accessors */
            bool connected();

            
        protected:

        private:
        
        /********************
         *      MEMBERS     *
         ********************/
        
        protected:

        private:
            
            bool     bIsConfigured;
            string   m_sDevicePath;
            uint32_t m_baud;
            uint16_t m_flowControl;
            uint16_t m_stopBits;
            uint16_t m_dataBits;
            uint16_t m_parity;

    };
}

#endif //__SERIAL_COMM_SOCKET_H_
