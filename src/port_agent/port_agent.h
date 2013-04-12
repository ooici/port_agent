#ifndef PORT_AGENT_H_
#define PORT_AGENT_H_

#include "common/daemon_process.h"
#include "network/tcp_comm_listener.h"
#include "network/tcp_comm_socket.h"
#include "connection/connection.h"
#include "config/port_agent_config.h"
#include "packet/packet.h"
#include "publisher/publisher_list.h"

#include <sys/select.h>
#include <time.h>

using namespace std;
using namespace packet;
using namespace network;
using namespace publisher;

#define SELECT_SLEEP_TIME 1

namespace port_agent {
    
    //////////////////////////////
    // Port Agent State
    typedef enum PortAgentState
    {
        STATE_UNKNOWN          = 0x00000000,
        STATE_STARTUP          = 0x00000001,
        STATE_UNCONFIGURED     = 0x00000002,
        STATE_CONFIGURED       = 0x00000003,
        STATE_CONNECTED        = 0x00000004,
        STATE_DISCONNECTED     = 0x00000005,
    } PortAgentState;
    
    class PortAgent : public DaemonProcess {
        public:
            PortAgent();
            PortAgent(int argc, char *argv[]);
            ~PortAgent();
            
            // virtual method from daemon process
            const string daemon_command();
            
            // Accessors
            const PortAgentState & getCurrentState() { return m_oState; };
            const string getCurrentStateAsString();
            
            bool start();
            void poll();
            string usage() { return PortAgentConfig::Usage(); }
            
        protected:
            // virtual method from daemon process
            const string pid_file();
            bool no_daemon();
            uint32_t ppid();
            float sleep_time() { return 0; }
            
        private:
            void setState(const PortAgentState &state);
            
            int buildFDSet(fd_set &readFDs);
            void processPortAgentCommands();
    
            void addObservatoryCommandListenerFD(int &maxFD, fd_set &readFDs);
            void addObservatoryCommandClientFD(int &maxFD, fd_set &readFDs);
            void addObservatoryDataListenerFD(int &maxFD, fd_set &readFDs);
            void addObservatoryStandardDataListenerFD(int &maxFD, fd_set &readFDs);
            void addObservatoryMultiDataListenerFDs(int &maxFD, fd_set &readFDs);
            void addObservatoryDataClientFD(int &maxFD, fd_set &readFDs);
            void addObservatoryStandardDataClientFD(int &maxFD, fd_set &readFDs);
            void addObservatoryMultiDataClientFDs(int &maxFD, fd_set &readFDs);
            void addInstrumentDataClientFD(int &maxFD, fd_set &readFDs);
            void addTelnetSnifferListenerFD(int &maxFD, fd_set &readFDs);
            
            int getObservatoryCommandListenerFD();
            int getObservatoryCommandClientFD();
            int getObservatoryDataListenerFD();
            int getObservatoryDataClientFD();
            int getInstrumentDataRxClientFD();
            int getInstrumentDataTxClientFD();
            int getTelnetSnifferListenerFD();
            
            void initializeObservatoryDataConnection();
            void initializeObservatoryStandardDataConnection();
            void initializeObservatoryMultiDataConnection();
            void initializeObservatoryCommandConnection();
            void initializeInstrumentConnection();
            void initializeTCPInstrumentConnection();
            void initialize_BOTPT_InstrumentConnection();
            void initializeSerialInstrumentConnection();
            bool initializeSerialSettings();
            
            // Publisher initializers
            void initializePublishers();
            void initializePublisherFile();
            void initializePublisherObservatoryData();    
            void initializePublisherObservatoryStandardData();
            void initializePublisherObservatoryMultiData();
            void initializePublisherObservatoryCommand();    
            void initializePublisherInstrumentData();    
            void initializePublisherInstrumentCommand();    
            void initializePublisherTelnetSniffer();    
            void initializePublisherTCP();    
            void initializePublisherUDP();    
            
            // State handlers
            void handleStateStartup();
            void handleStateUnconfigured(const fd_set &readFDs);
            void handleStateConfigured(const fd_set &readFDs);
            void handleStateConnected(const fd_set &readFDs);
            void handleStateDisconnected(const fd_set &readFDs);
            void handleCommon(const fd_set &readFDs);
            void handleStateUnknown();
            
            // Other handlers
            void handlePortAgentCommand(const char *commands);
            void handleTCPConnect(TCPCommListener &listener);
            
            void handleTelnetSnifferAccept(const fd_set &readFDs);
            void handleObservatoryCommandAccept(const fd_set &readFDs);
            void handleObservatoryCommandRead(const fd_set &readFDs);
            void handleObservatoryDataAccept(const fd_set &readFDs);
            void handleObservatoryStandardDataAccept(const fd_set &readFDs);
            void handleObservatoryMultiDataAccept(const fd_set &readFDs);
            void handleObservatoryDataRead(const fd_set &readFDs);
            void handleObservatoryStandardDataRead(const fd_set &readFDs);
            void handleObservatoryMultiDataRead(const fd_set &readFDs);
            void handleInstrumentDataRead(const fd_set &readFDs);
            
            void publishHeartbeat();
            void publishFault(const string &msg);
            void publishStatus(const string &msg);
            void publishPacket(Packet *packet);
            void publishPacket(char *payload, uint16_t size, PacketType type);

            void displayVersion();
            void setRotationInterval();
            
        /////
        // Members
        /////
        
        protected:
            
        private:
            PortAgentConfig *m_pConfig;
            PortAgentState  m_oState;
            
            PublisherList m_oPublishers;
            time_t m_lLastHeartbeat;
            
            // Port agent connections
            Connection *m_pObservatoryConnection;
            Connection *m_pInstrumentConnection;
            
            // Publisher Connections
            TCPCommListener *m_pTelnetSnifferConnection;
            
    };
}

#endif //PORT_AGENT_H
