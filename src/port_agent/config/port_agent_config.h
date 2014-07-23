/*******************************************************************************
 * Class: PortAgentConfig
 * Filename: port_agent_config.h
 * Author: Bill French (wfrench@ucsd.edu)
 * License: Apache 2.0
 *
 * Configuration object for Port Agents.
 *
 * This class:
 *   * parses options from argc and argv
 *     passed to the constructor using getopt_long.
 *   * parses options read from the observatory command interface
 *   * store configuration parameters
 ******************************************************************************/

#ifndef PORT_AGENT_CONFIG_H_
#define PORT_AGENT_CONFIG_H_

#include <string>
#include <list>
#include <stdint.h>
#include "common/log_file.h"

using namespace std;
using namespace logger;

#define DEFAULT_PACKET_SIZE   1024
#define DEFAULT_BREAK_DURATION 100
#define MIN_BREAK_DURATION 100
#define MAX_BREAK_DURATION 4000
#define MAX_PACKET_SIZE       4097
#define RSN_RAW_PACKET_BUFFER_SIZE 65536  // TODO: What should RSN packet buffer size be?
#define DEFAULT_HEARTBEAT_INTERVAL 120

// Set the RSN Digi to add Binary Timestamps to data
#define TIMESTAMP_BINARY 2

#define BASE_FILENAME "port_agent"

#define DEFAULT_LOG_DIR   "/tmp"
#define DEFAULT_CONF_DIR  "/tmp"
#define DEFAULT_PID_DIR   "/tmp"
#define DEFAULT_DATA_DIR  "/tmp"


namespace port_agent {
    //////////////////////////////
    // Port Agent Commands
    typedef enum PortAgentCommand
    {
        CMD_UNKNOWN                 = 0x00000000,
        CMD_HELP                    = 0x00000001,
        CMD_COMM_CONFIG_UPDATE      = 0x00000002,
        CMD_PUBLISHER_CONFIG_UPDATE = 0x00000003,
        CMD_PATH_CONFIG_UPDATE      = 0x00000004,
        CMD_SAVE_CONFIG             = 0x00000005,
        CMD_GET_CONFIG              = 0x00000006,
        CMD_GET_STATE               = 0x00000007,
        CMD_PING                    = 0x00000008,
        CMD_BREAK                   = 0x00000009,
        CMD_SHUTDOWN                = 0x00000010,
        CMD_ROTATION_INTERVAL       = 0x00000011
    } PortAgentCommand;
    typedef list<PortAgentCommand>  CommandQueue;
    
    typedef enum ObservatoryConnectionType
    {
        OBS_TYPE_UNKNOWN       = 0x00000000,
        OBS_TYPE_STANDARD      = 0x00000001,
        OBS_TYPE_MULTI         = 0x00000002,
    } ObservatoryConnectionType;

    typedef enum InstrumentConnectionType
    {
        TYPE_UNKNOWN           = 0x00000000,
        TYPE_SERIAL            = 0x00000001,
        TYPE_TCP               = 0x00000002,
        TYPE_BOTPT             = 0x00000003,
        TYPE_RSN               = 0x00000004
    } InstrumentConnectionType;

    // DHE NEW: a list of data port entries; in the future the ObservatoryDataPortEntry_T
    // can be extended to be a structure including a routing key.  Also, the fact that
    // it's a list should be abstracted, so that we can change it to a map for faster
    // lookup in the future.
    typedef int ObservatoryDataPortEntry_T;
    typedef list<ObservatoryDataPortEntry_T> ObservatoryDataPorts_T;
    
    // A singleton class that contains observatory data ports, and provides operations
    // such as add, delete, etc.
    class ObservatoryDataPorts {
        public:
            static  ObservatoryDataPorts* instance();
            void    logPorts();
            bool    addPort(const int port);
            const int getFirstPort();
            const int getNextPort();

        private:
            // CTOR
            ObservatoryDataPorts();

            static ObservatoryDataPorts* m_pInstance;
            ObservatoryDataPorts_T        m_observatoryDataPorts;
            ObservatoryDataPorts_T::iterator m_portIt;
    };

    class PortAgentConfig {
        public:
            ///////////////////////
            // Constructors
            PortAgentConfig() {}
            PortAgentConfig(int argc, char *argv[]);
            
            ///////////////////////
            // Public Methods
            
            ///////////////////////
            // Accessors
            static string Usage();
            PortAgentCommand getCommand();
            
            // Commands
            void saveConfig();
            string getConfig();
            bool readConfig(const string & filename);
            
            bool isConfigured();
            
            bool parse(const string &commands);
            
            // Set methods
            bool setObservatoryConnectionType(const string &param);
            bool setObservatoryDataPort(const string &param);
            // DHE: new name for now; might be called setObservatoryDataPort
            // when replaces, but that name isn't as descriptive
            bool addObservatoryDataPort(const string &param);
            bool setObservatoryCommandPort(const string &param);
            bool setInstrumentBreakDuration(const string &param);
            bool setInstrumentConnectionType(const string &param);
            bool setSentinleSequence(const string &param);
            bool setOutputThrottle(const string &param);
            bool setHeartbeatInterval(const string &param);
            bool setMaxPacketSize(const string &param);
            bool setLogLevel(const string &param);
            bool setDevicePath(const string &param);
            bool setBaud(const string &param);
            bool setStopbits(const string &param);
            bool setDatabits(const string &param);
            bool setParity(const string &param);
            bool setFlow(const string &param);
            bool setInstrumentDataPort(const string &param);
            bool setInstrumentDataTxPort(const string &param);
            bool setInstrumentDataRxPort(const string &param);
            bool setInstrumentCommandPort(const string &param);
            bool setRotationInterval(const string &param);
			bool setTelnetSnifferPort(const string &param);
            bool setTelnetSnifferPrefix(const string &param) { m_telnetSnifferPrefix = param; return true; }
            bool setTelnetSnifferSuffix(const string &param) { m_telnetSnifferSuffix = param; return true; }
            
            // Common Config
            string programName() { return m_programName; }
            bool help() { return m_help; }
            bool kill() { return m_kill; }
            bool version() { return m_version; }
            uint32_t ppid() { return m_ppid; }
            
            string logfile();
            string pidfile();
            string conffile();
            string datafile();
            
            string logdir() { return m_logdir; }
            string piddir() { return m_piddir; }
            string confdir() { return m_confdir; }
            string datadir() { return m_datadir; }
            
			RotationType rotation_interval() { return m_eRotationInterval; }
            
            bool noDetatch() { return m_noDetatch; }
            unsigned short verbose() { return m_verbose; }
            unsigned int observatoryCommandPort() { return m_observatoryCommandPort; }
            unsigned int observatoryDataPort() { return m_observatoryDataPort; }
            
            ObservatoryConnectionType observatoryConnectionType() { return m_observatoryConnectionType; }
            InstrumentConnectionType instrumentConnectionType() { return m_instrumentConnectionType; }
            const string & sentinleSequence() { return m_sentinleSequence; }
            uint32_t outputThrottle() { return m_outputThrottle; }
            uint32_t heartbeatInterval() { return m_heartbeatInterval; }
            uint32_t maxPacketSize() { return m_maxPacketSize; }
            
            bool    devicePathChanged() { return m_bDevicePathChanged; }
            void    clearDevicePathChanged() { m_bDevicePathChanged = false; }
            bool    serialSettingsChanged() { return m_bSerialSettingsChanged; }
            void    clearSerialSettingsChanged() { m_bSerialSettingsChanged = false; }
            const string & devicePath() { return m_devicePath; }
            uint32_t breakDuration() { return m_breakDuration; }
            uint32_t baud() { return m_baud; }
            uint16_t stopbits() { return m_stopbits; }
            uint16_t databits() { return m_databits; }
            uint16_t parity() { return m_parity; }
            uint16_t flow() { return m_flow; }
            const string & instrumentAddr() { return m_instrumentAddr; }
            uint16_t instrumentDataPort() { return m_instrumentDataPort; }
            uint16_t instrumentDataTxPort() { return m_instrumentDataTxPort; }
            uint16_t instrumentDataRxPort() { return m_instrumentDataRxPort; }
            uint16_t instrumentCommandPort() { return m_instrumentCommandPort; }
			
			// Telnet sniffer config
            uint16_t telnetSnifferPort() { return m_telnetSnifferPort; }
            string telnetSnifferPrefix() { return m_telnetSnifferPrefix; }
            string telnetSnifferSuffix() { return m_telnetSnifferSuffix; }
            
        private:
            void setParameter(char option, char *value);
            void addCommand(PortAgentCommand command);
            bool processCommand(const string & command);
            bool splitCommand(const string & raw, string & cmdResult, string & parameter);
            
            void verifyCommandLineParameters();
            
            ///////////////////////
            // Private Data
            
            // storage for the commands processed by this object
            CommandQueue m_commands;
            
            // Command line options, not all of these can be changed via
            // public methods post construction.
            bool m_help;
            bool m_kill;
            bool m_version;
            string m_programName;
			uint32_t m_ppid;
            
            string m_pidfile;
            string m_logfile;
            string m_conffile;
            
            string m_piddir;
            string m_logdir;
            string m_confdir;
            string m_datadir;
            
            bool m_noDetatch;
            unsigned short m_verbose;
            
            uint16_t m_observatoryCommandPort;
            uint16_t m_observatoryDataPort;
            string m_sentinleSequence;
            
            uint32_t m_outputThrottle;
            uint32_t m_maxPacketSize;
            
            ObservatoryConnectionType m_observatoryConnectionType;
            InstrumentConnectionType m_instrumentConnectionType;
            RotationType m_eRotationInterval;
			
            uint16_t m_heartbeatInterval;
			
			bool    m_bDevicePathChanged;
            bool    m_bSerialSettingsChanged;
            string  m_devicePath;
            uint32_t m_breakDuration;
            uint32_t m_baud;
            uint16_t m_stopbits;
            uint16_t m_databits;
            uint16_t m_parity;
            uint16_t m_flow;
            string m_instrumentAddr;
            uint16_t m_instrumentDataPort;
            uint16_t m_instrumentDataTxPort;
            uint16_t m_instrumentDataRxPort;
            uint16_t m_instrumentCommandPort;
			
			// Telnet sniffer config
			uint16_t m_telnetSnifferPort;
			string m_telnetSnifferPrefix;
			string m_telnetSnifferSuffix;
    };
}


#endif //PORT_AGENT_CONFIG_H_
