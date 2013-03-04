/*******************************************************************************
 * Class: DriverCommandPublisher
 * Filename: Driver_command_publisher.h
 * Author: Bill French (wfrench@ucsd.edu)
 * License: Apache 2.0
 *
 * Write packet to the driver command port.
 *    
 ******************************************************************************/

#ifndef __DRIVER_COMMAND_PUBLISHER_H_
#define __DRIVER_COMMAND_PUBLISHER_H_

#include "driver_publisher.h"
#include "common/log_file.h"

using namespace std;
using namespace logger;

namespace publisher {
    class DriverCommandPublisher : public DriverPublisher {
        /********************
         *      METHODS     *
         ********************/
        
        public:
           DriverCommandPublisher();
           DriverCommandPublisher(CommBase *socket) : DriverPublisher(socket) {}

	   const PublisherType publisherType() { return PUBLISHER_DRIVER_COMMAND; }
	   
        protected:
            virtual bool handleInstrumentData(Packet *packet)     { return logPacket(packet); }
            virtual bool handleDriverData(Packet *packet)         { return logPacket(packet); }
            // DHE: I had to change this because the command port is ephemeral; can't publish to
            // it unless the code handles broken pipe better.
            //virtual bool handleCommand(Packet *packet)            { return logPacket(packet); }
            virtual bool handleCommand(Packet *packet)            { return true; }
            virtual bool handleStatus(Packet *packet)             { return logPacket(packet); }
            virtual bool handleFault(Packet *packet)              { return logPacket(packet); }
            virtual bool handleDriverCommand(Packet *packet)      { return logPacket(packet); }
            virtual bool handleHearteat(Packet *packet)           { return logPacket(packet); }

        private:
        
        /********************
         *      MEMBERS     *
         ********************/
        
        protected:
            
        private:

    };
}

#endif //__DRIVER_COMMAND_PUBLISHER_H_
