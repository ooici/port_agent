/*******************************************************************************
 * Class: PortAgentConfig
 * Filename: port_agent_config.cpp
 * Author: Bill French (wfrench@ucsd.edu)
 * License: Apache 2.0
 *
 * Configuration object for Port Agents.  This class parses options from argv
 * passed to the constructor using getopt_long. 
 ******************************************************************************/
#include "port_agent_config.h"
#include "common/logger.h"
#include "common/log_file.h"
#include "common/exception.h"
#include "common/util.h"

#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <list>
#include <algorithm>

using namespace std;
using namespace logger;
using namespace port_agent;

/******************************************************************************
 *   PUBLIC METHODS
 ******************************************************************************/

/******************************************************************************
 * Method: Constructor
 * Description: Construct a configuration object from command line parameters
 *              passed in from the command line using (argv).
 ******************************************************************************/
PortAgentConfig::PortAgentConfig(int argc, char* argv[]) {
    
    int c = 0;
    
    
    if(argc) 
        m_programName = string(argv[0]);
    
    LOG(INFO) << "PortAgentConfig ctor";
    
    // Initalize Defaults
    m_noDetatch = false;
    m_verbose = 0;
    m_observatoryCommandPort = 0;
    m_observatoryDataPort = 0;
    m_help = false;
    m_kill = false;
    m_version = false;
    m_outputThrottle = 0;
    m_maxPacketSize = DEFAULT_PACKET_SIZE;
    m_ppid = 0;
    m_telnetSnifferPort = 0;
    
    // For backward compatibility, observatory connection defaults to standard
    m_observatoryConnectionType = OBS_TYPE_STANDARD;

    m_instrumentConnectionType = TYPE_UNKNOWN;
    
    // Connection settings
    m_bDevicePathChanged = true;
    m_bSerialSettingsChanged = true;
    m_baud = 0;
    m_stopbits = 1;
    m_databits = 8;
    m_parity = 0;
    m_flow = 0;
    m_instrumentDataPort = 0;
    m_instrumentDataTxPort = 0;
    m_instrumentDataRxPort = 0;
    m_instrumentCommandPort = 0;
    m_heartbeatInterval = DEFAULT_HEARTBEAT_INTERVAL;
    
    m_piddir = DEFAULT_PID_DIR;
    m_logdir = DEFAULT_LOG_DIR;
    m_confdir = DEFAULT_CONF_DIR;
    m_datadir = DEFAULT_DATA_DIR;
            
    // the getopt string representing command line options.
    string optstr = "y:u:c:vhsp:k";
        
    // Long option equiviant of getopt string.
    static struct option long_options[] = {
        {"conffile",  required_argument, 0,  'c' },
        {"verbose",   no_argument, 0,  'v' },
        {"help",      no_argument, 0,  'h' },
        {"kill",      no_argument, 0,  'k' },
        {"single",    no_argument, 0,  's' },
        {"version",   no_argument, 0, 'n' },
        {"ppid",      required_argument, 0,  'y' },
        
        {"command_port",  required_argument, 0,  'p' },
        {NULL,         0,                 NULL,  0 }
    };

    LOG(DEBUG) << "Arg Count: " << argc << " Calculated Count: " << sizeof(argv) / sizeof(char*);
    
    // Reset optind so that this can be called more than once in a program.
    // Really only matters for unit tests
    optind = 1;
    
    // Parse the command line options and store their values.
    do {
        int option_index = 0;
        c = getopt_long(argc, argv, optstr.c_str(),
                        long_options, &option_index);
        
        if (c > 0) {
            if(optarg)
                LOG(DEBUG) << "SET: " << c << " VALUE: " << optarg;
            else
                LOG(DEBUG) << "SET: " << c;
            setParameter(c, optarg);
        }
    }
    while (c > 0);

    LOG(DEBUG) << "CONFIG: " << getConfig();
    verifyCommandLineParameters();
}


/******************************************************************************
 * Method: conffile()
 * Description: return a path to the configuration file;
 * Return: formatted path string
 ******************************************************************************/
string PortAgentConfig::conffile() {
    ostringstream out;
    out << confdir() << "/" << BASE_FILENAME << "_"
        << observatoryCommandPort() << ".conf";
    
    LOG(DEBUG) << "Config path: " << out.str();
    
    return out.str();
}

/******************************************************************************
 * Method: database()
 * Description: return a path to the data file;
 * Return: formatted path string
 ******************************************************************************/
string PortAgentConfig::datafile() {
    ostringstream out;
    out << logdir() << "/" << BASE_FILENAME << "_"
        << observatoryCommandPort();
    
    LOG(DEBUG) << "Data file: " << out.str();
    
    return out.str();
}



/******************************************************************************
 * Method: logfile()
 * Description: return a path to the log file;
 * Return: formatted path string
 ******************************************************************************/
string PortAgentConfig::logfile() {
    ostringstream out;
    out << logdir() << "/" << BASE_FILENAME << "_"
        << observatoryCommandPort() << ".log";
    
    LOG(DEBUG) << "Log path: " << out.str();
    
    return out.str();
}


/******************************************************************************
 * Method: pidfile()
 * Description: return a path to the pid file;
 * Return: formatted path string
 ******************************************************************************/
string PortAgentConfig::pidfile() {
    ostringstream out;

    if(m_pidfile.length())
        return m_pidfile;

    out << piddir() << "/" << BASE_FILENAME << "_"
        << observatoryCommandPort() << ".pid";
    
    LOG(DEBUG) << "Pid path: " << out.str();
    
    return out.str();
}


/******************************************************************************
 * Method: usage()
 * Description: Return a formatted string containing the valid options for
 *              a port agent.
 * Return: formatted usage string
 ******************************************************************************/
string PortAgentConfig::Usage() {
    ostringstream os;
    os << "USAGE: " << "port_agent" << endl
       << "\t" << " --help" 
               << "\t\t\t- Display this message " << endl
       << "\t" << " --version" 
               << "\t\t\t- Display the port agent version " << endl
       << "\t" << " --kill" 
               << "\t\t\t- Kill a daemon processes associated to a command port " << endl
       << "\t" << " --verbose (-v) " 
               << "\t- Increase program verbosity " << endl << endl
               
       << "\t" << " --conffile (-c) config_file "
               << "\t- Path to port_agent config file" << endl
       
       << "\t" << " --command_port (-p) port" 
               << "\t- Observatory command port number " << endl
       
       << "\t" << " --ppid (-y) parent_process_id" 
               << "\t- Poison pill, if parent process is gone then shutdown " << endl
       
       << "\t" << " --single (-s)" 
               << "\t- Run in single thread mode. Do not detatch " << endl;
       
    return os.str();
}

/******************************************************************************
 * Method: getCommand()
 * Description: Return a command from the FIFO command queue.
 * Return: The oldest command if there are any in the queue, otherwise return
 *         0.
 ******************************************************************************/
PortAgentCommand PortAgentConfig::getCommand() {
    if(m_commands.empty()) {
        return CMD_UNKNOWN;
    } else {
        PortAgentCommand cmd = m_commands.front();
        m_commands.pop_front();
        return cmd;
    }
}

/******************************************************************************
 * Method: saveConfig()
 * Description: Return a command from the FIFO command queue.
 * Return: The oldest command if there are any in the queue, otherwise return
 *         0.
 ******************************************************************************/
void PortAgentConfig::saveConfig() {
    string configFile = conffile();
    
}

/******************************************************************************
 * Method: parse()
 * Description: parse a string of characters into commands.  If they are
 *              set parameters then also set the appropriate member variable.
 * Return: return true if all bytes were successfully parsed into config
 *         commands.
 ******************************************************************************/
bool PortAgentConfig::parse(const string &commands) {
    istringstream iss(commands);
    char buffer[1024];
    
    while(iss.getline(buffer, 1024)) {
        string cmd(buffer);
        chomp(cmd);
        LOG(DEBUG) << "Config CMD: " << cmd;
        
        if(!processCommand(cmd)) {
            LOG(ERROR) << "failed to parse: " << cmd;
            return false;
        }
    }
    
    return true;
}

/******************************************************************************
 * Method: isConfigured()
 * Description: determine if we have enough information to run the port agent.
 * Return: return true we are ready for a port agent state change
 ******************************************************************************/
bool PortAgentConfig::isConfigured() {
    bool ready = true;
    
    if(! instrumentConnectionType()) {
        LOG(DEBUG) << "Missing instrument connection type";
        ready = false;
    }
    
    if(! observatoryCommandPort()) {
        LOG(DEBUG) << "Missing observatory command port";
        ready = false;
    }

    if(! observatoryDataPort()) {
        LOG(DEBUG) << "Missing observatory data port";
        ready = false;
    }

    if(instrumentConnectionType() == TYPE_TCP ||
       instrumentConnectionType() == TYPE_RSN) {
        if(! instrumentAddr().length()) {
            LOG(DEBUG) << "Missing instrument address";
            ready = false;
        }
        
        if(! instrumentDataPort()) {
            LOG(DEBUG) << "Missing instrument data port";
            ready = false;
        }
    }

    if(instrumentConnectionType() == TYPE_BOTPT) {
        if(! instrumentAddr().length()) {
            LOG(DEBUG) << "Missing instrument address";
            ready = false;
        }

        if(! instrumentDataTxPort()) {
            LOG(DEBUG) << "Missing instrument data TX port";
            ready = false;
        }

        if(! instrumentDataRxPort()) {
            LOG(DEBUG) << "Missing instrument data RX port";
            ready = false;
        }
    }
    
    if(instrumentConnectionType() == TYPE_RSN) {
        if(! instrumentCommandPort()) {
            LOG(DEBUG) << "Missing instrument command port";
            ready = false;
        }
    }
    
    if(instrumentConnectionType() == TYPE_SERIAL) {
        if(! baud()) {
            LOG(DEBUG) << "Missing baud rate";
            ready = false;
        }
    }
    
    return ready;
}

/******************************************************************************
 * Method: readConfig()
 * Description: Read a config file and store the content in this object
 * Return: return true if all lines were read successfully
 ******************************************************************************/
bool PortAgentConfig::readConfig(const string & filename) {
    ifstream infile(filename.c_str());
    char buffer[128];
    bool success = true;
    
    if(!infile)
        throw FileIOException(filename);
    
    LOG(DEBUG) << "Reading config from file: " << filename;
    while(infile.getline(buffer, 128)) {
        if(! parse(buffer) )
            success = false;     
    }
    
    return success;
}

/******************************************************************************
 * Method: getConfig()
 * Description: return a string dump of the current configuration which can
 *              be read in by the port agent to return state.
 * Return: string dump of the current config
 ******************************************************************************/
string PortAgentConfig::getConfig() {
    ostringstream out;
    const char *buffer;
    string loglevel = Logger::Instance()->levelToString(Logger::GetLogLevel());

    out << "pid_dir " << m_piddir << endl
        << "log_dir " << m_logdir << endl
        << "conf_dir " << m_confdir << endl
        << "data_dir " << m_datadir << endl
        
        << "log_level " << loglevel << endl
        
        << "command_port " << m_observatoryCommandPort << endl
        << "data_port " << m_observatoryDataPort << endl;
        
        if(m_instrumentConnectionType) {
            out << "instrument_type ";
            
            if(m_instrumentConnectionType == TYPE_SERIAL)
                out << "serial";
            else if(m_instrumentConnectionType == TYPE_TCP)
                out << "tcp";
            else if(m_instrumentConnectionType == TYPE_BOTPT)
                out << "BOTPT";
            else if(m_instrumentConnectionType == TYPE_RSN)
                out << "rsn";
            
            out << endl;
        }
        
        out << "heartbeat_interval " << m_heartbeatInterval << endl;
        
        buffer = m_sentinleSequence.c_str(); 
        out << "sentinle '";
        for(int i = 0; i < m_sentinleSequence.length(); i++) {
            if(buffer[i] == '\n')
                out << "\\n";
            else if(buffer[i] == '\r')
                out << "\\r";
            else
                out << buffer[i];
        }
        out << "'" << endl;
        
        out << "output_throttle " << m_outputThrottle << endl
            << "max_packet_size " << m_maxPacketSize << endl
            << "baud " << m_baud << endl
            << "stopbits " << m_stopbits << endl
            << "databits " << m_databits << endl
            << "parity " << m_parity << endl
            << "flow " << m_flow << endl
            << "instrument_addr " << m_instrumentAddr << endl
            << "instrument_data_port " << m_instrumentDataPort << endl
            << "instrument_data_tx_port " << m_instrumentDataTxPort << endl
            << "instrument_data_rx_port " << m_instrumentDataRxPort << endl
            << "instrument_command_port " << m_instrumentCommandPort << endl;
            
        if(m_telnetSnifferPort) {
            out << "telnet_niffer_port " << m_telnetSnifferPort << endl;
            if(m_telnetSnifferPrefix.length()) 
                out << "telnet_sniffer_prefix " << m_telnetSnifferPrefix << endl;
            if(m_telnetSnifferSuffix.length()) 
                out << "telnet_sniffer_suffix " << m_telnetSnifferSuffix << endl;
        }
        
    return out.str();
}


//////
// Set Methods
//////
/******************************************************************************
 * Method: setInstrumentBreakDuration
 * Description: Set the break duration
 * Return:
 *     return true if the type was set correctly, otherwise false for unknown
 *     types.
 *****************************************************************************/
bool PortAgentConfig::setInstrumentBreakDuration(const string &param) {
    m_breakDuration = DEFAULT_BREAK_DURATION;

    if (0 == param.size()) {
        LOG(INFO) << "break duration not specified; using " << DEFAULT_BREAK_DURATION;
        m_breakDuration = DEFAULT_BREAK_DURATION;
    }
    else {
        const char* v = param.c_str();
        int value = atoi(v);
        if (value < 0) {
            LOG(INFO) << "attempt to set break duration to a negative.  using default.";
            value = DEFAULT_BREAK_DURATION;
        }
        m_breakDuration = value;
    }

    LOG(INFO) << "set break duration to " << m_breakDuration;
    return true;
}

/******************************************************************************
 * Method: setInstrumentConnectionType
 * Description: Set the configuration instrument connection type
 * Return:
 *     return true if the type was set correctly, otherwise false for unknown
 *     types.
 *****************************************************************************/
bool PortAgentConfig::setInstrumentConnectionType(const string &param) {
    m_instrumentConnectionType = TYPE_UNKNOWN;
    
    if(param == "serial") {
        LOG(INFO) << "connection type set to serial";
        m_instrumentConnectionType = TYPE_SERIAL;
    }
    
    else if(param == "tcp") {
        LOG(INFO) << "connection type set to tcp";
        m_instrumentConnectionType = TYPE_TCP;
    }
    
    else if(param == "botpt") {
        LOG(INFO) << "connection type set to botpt";
        m_instrumentConnectionType = TYPE_BOTPT;
    }

    else if(param == "rsn") {
        LOG(INFO) << "connection type set to rsn";
        m_instrumentConnectionType = TYPE_RSN;
    }
    
    else {
        LOG(ERROR) << "unknown connection type: " << param;
        m_instrumentConnectionType = TYPE_UNKNOWN;
        return false;
    }
    
    return true;
}

/******************************************************************************
 * Method: setObservatoryConnectionType
 * Description: Set the configuration observatory connection type
 * Return:
 *     return true if the type was set correctly. Currently if type unknown,
 *     printing error, returning false.
 *****************************************************************************/
bool PortAgentConfig::setObservatoryConnectionType(const string &param) {

    if (param == "multi") {
        LOG(INFO) << "observatory connection type set to multi";
        m_observatoryConnectionType = OBS_TYPE_MULTI;
    }
    else if (param == "standard") {
        LOG(INFO) << "observatory connection type set to standard";
        m_observatoryConnectionType = OBS_TYPE_STANDARD;
    }
    else {
        LOG(ERROR) << "unknown observatory connection type: " << param;
        m_observatoryConnectionType = OBS_TYPE_UNKNOWN;
        return false;
    }

    return true;
}

/******************************************************************************
 * Method: setSentinleSequence
 * Description: Set the sentinle sequence
 * Param:
 *     command - the entire command string to set the sentinle.  We do this
 *     because we need to do custom parsing for this command in case there is
 *     a CR or LF embedded in the sentinle. 
 * Return:
 *     return true if the sequence was set correctly, otherwise false.
 *****************************************************************************/
bool PortAgentConfig::setSentinleSequence(const string &command) {
    istringstream iss(command);
    
    string cmd, param, dump;
    m_sentinleSequence = param;
    
    char buffer[128];
    
    iss >> cmd;
    iss.getline(buffer, 128, '\'');
    
    // if buffer is empty then no single quote was found.
    if(!buffer[0]) {
        LOG(ERROR) << "Failed to parse sentinle string: " << command;
        return false;
    }
    
    iss.getline(buffer, 128, '\'');
    
    // Iterate through the buffer and substitute \n and \r strings
    for(int i = 0; i < 128 && buffer[i]; i++) {
        if(buffer[i] == '\\') {
            if( i >= 127 || (buffer[i+1] != 'n' && buffer[i+1] != 'r') ) {
                LOG(DEBUG) << "Sentinle backslash ignored";
                param += buffer[i];
            }
            else if(buffer[i+1] == 'n') {
                LOG(DEBUG) << "Sentinle sub: \n";
                param += "\n";
                i++;
            }
            else if(buffer[i+1] == 'r') {
                LOG(DEBUG) << "Sentinle sub: \r";
                param += "\r";
                i++;
            }
        } else {
            LOG(DEBUG) << "add sentinle char: " << buffer[i];
            param += buffer[i];
        }
    }
    
    LOG(DEBUG) << "Sentinle string length: " << param.length();
    
    m_sentinleSequence = param;
    
    return true;
}

/******************************************************************************
 * Method: setOutputThrottle
 * Description: Set the output throttle
 * Param:
 *     param - string represention of the value of the throttle.  If it is not
 *     a number the value will be set to 0.
 * Return:
 *     return true if the throttle was set correctly, otherwise false.
 *****************************************************************************/
bool PortAgentConfig::setOutputThrottle(const string &param) {
    const char* v = param.c_str();
    
    int value = atoi(v);
    m_outputThrottle = 0;
    
    if(value == 0 && v[0] != '0') {
        LOG(ERROR) << "invalid output throttle parameter, " << param;
        return false;
    }
    
    if(value < 0) {
        LOG(ERROR) << "attempt to set output throttle to a negative.  0 instead.";
        return false;
    }
    
    LOG(INFO) << "set output throttle to " << value;
    m_outputThrottle = value;
    return true;
}

/******************************************************************************
 * Method: setHeartbeatInterval
 * Description: Set the heartbeat interval
 * Param:
 *     param - string represention of the value of the throttle.  If it is not
 *     a number the value will be set to 0.
 * Return:
 *     return true if the throttle was set correctly, otherwise false.
 *****************************************************************************/
bool PortAgentConfig::setHeartbeatInterval(const string &param) {
    const char* v = param.c_str();
    
    int value = atoi(v);
    
    if(value == 0 && v[0] != '0') {
        LOG(ERROR) << "invalid heartbeat interval parameter, " << param;
        return false;
    }
    
    if(value < 0) {
        LOG(ERROR) << "attempt to set heartbeat interval to a negative.";
        m_heartbeatInterval = 0;
        return false;
    }
    
    LOG(INFO) << "set heartbeat interval to " << value;
    m_heartbeatInterval = value;
    return true;
}

/******************************************************************************
 * Method: setObervatoryDataPort
 * Description: Set the observatory data port
 * Param:
 *     param - string represention of the value of the port.  If it is not
 *     a number the value will be set to 0.
 * Return:
 *     return true if the throttle was set correctly, otherwise false.
 *****************************************************************************/
bool PortAgentConfig::setObservatoryDataPort(const string &param) {
    const char* v = param.c_str();
    
    int value = atoi(v);
    m_observatoryDataPort = 0;
    
    if(value <= 0 || value > 65535) {
        LOG(ERROR) << "Invalid port specification, setting to 0";
        return false;
    }
    
    LOG(INFO) << "set observatory data port to " << value;
    m_observatoryDataPort = value;
    return true;
}

/******************************************************************************
 * Method: addObervatoryDataPort
 * Description: Add the given observatory data port
 * Param:
 *     param - string represention of the value of the port.  If it is not
 *     a number the value will be set to 0.
 * Return:
 *     return true if the throttle was set correctly, otherwise false.
 *****************************************************************************/
// DHE: there should be an object that encapsulates a list or map (or whatever)
// of data port entries (which could include routing keys).  The object is
// a singleton; the observatory data port container.
bool PortAgentConfig::addObservatoryDataPort(const string &param) {
    const char* v = param.c_str();

    int value = atoi(v);
    m_observatoryDataPort = 0;

    if(value <= 0 || value > 65535) {
        LOG(ERROR) << "Invalid port specification, setting to 0";
        return false;
    }

    LOG(INFO) << "adding observatory data port: " << value;
    //m_pObservatoryDataPorts->add(value);

    // DHE NEW: for now keep this
    m_observatoryDataPort = value;

    if (false == ObservatoryDataPorts::instance()->addPort(value)) {
        return false;
    }

    // DHE TEMPTEMP
    ObservatoryDataPorts::instance()->logPorts();

    return true;
}

/******************************************************************************
 * Method: setObervatoryCommandPort
 * Description: Set the observatory command port
 * Param:
 *     param - string represention of the value of the port.  If it is not
 *     a number the value will be set to 0.
 * Return:
 *     return true if the throttle was set correctly, otherwise false.
 *****************************************************************************/
bool PortAgentConfig::setObservatoryCommandPort(const string &param) {
    const char* v = param.c_str();
    
    int value = atoi(v);
    m_observatoryCommandPort = 0;
    
    if(value <= 0 || value > 65535) {
        LOG(ERROR) << "Invalid port specification, setting to 0";
        return false;
    }
    
    LOG(INFO) << "set observatory command port to " << value;
    m_observatoryCommandPort = value;
    return true;
}

/******************************************************************************
 * Method: setInstrumentDataPort
 * Description: Set the instrument data port
 * Param:
 *     param - string represention of the value of the port.  If it is not
 *     a number the value will be set to 0.
 * Return:
 *     return true if the port was set correctly, otherwise false.
 *****************************************************************************/
bool PortAgentConfig::setInstrumentDataPort(const string &param) {
    const char* v = param.c_str();
    
    int value = atoi(v);
    m_instrumentDataPort = 0;
    
    if(value <= 0 || value > 65535) {
        LOG(ERROR) << "Invalid port specification, setting to 0";
        return false;
    }
    
    LOG(INFO) << "set instrument data port to " << value;
    m_instrumentDataPort = value;
    return true;
}

/******************************************************************************
 * Method: setInstrumentDataTxPort
 * Description: Set the instrument TX data port (BOTPT)
 * Param:
 *     param - string represention of the value of the port.  If it is not
 *     a number the value will be set to 0.
 * Return:
 *     return true if the port was set correctly, otherwise false.
 *****************************************************************************/
bool PortAgentConfig::setInstrumentDataTxPort(const string &param) {
    const char* v = param.c_str();

    int value = atoi(v);
    m_instrumentDataTxPort = 0;

    if(value <= 0 || value > 65535) {
        LOG(ERROR) << "Invalid port specification, setting to 0";
        return false;
    }

    LOG(INFO) << "set instrument data port to " << value;
    m_instrumentDataTxPort = value;
    return true;
}

/******************************************************************************
 * Method: setInstrumentDataRxPort
 * Description: Set the instrument RX data port (BOTPT)
 * Param:
 *     param - string represention of the value of the port.  If it is not
 *     a number the value will be set to 0.
 * Return:
 *     return true if the port was set correctly, otherwise false.
 *****************************************************************************/
bool PortAgentConfig::setInstrumentDataRxPort(const string &param) {
    const char* v = param.c_str();

    int value = atoi(v);
    m_instrumentDataRxPort = 0;

    if(value <= 0 || value > 65535) {
        LOG(ERROR) << "Invalid port specification, setting to 0";
        return false;
    }

    LOG(INFO) << "set instrument data port to " << value;
    m_instrumentDataRxPort = value;
    return true;
}

/******************************************************************************
 * Method: setInstrumentCommandPort
 * Description: Set the instrument command port
 * Param:
 *     param - string represention of the value of the port.  If it is not
 *     a number the value will be set to 0.
 * Return:
 *     return true if the port was set correctly, otherwise false.
 *****************************************************************************/
bool PortAgentConfig::setInstrumentCommandPort(const string &param) {
    const char* v = param.c_str();
    
    int value = atoi(v);
    m_instrumentCommandPort = 0;
    
    if(value <= 0 || value > 65535) {
        LOG(ERROR) << "Invalid port specification, setting to 0";
        return false;
    }
    
    LOG(INFO) << "set instrument command port to " << value;
    m_instrumentCommandPort = value;
    return true;
}

/******************************************************************************
 * Method: setMaxPacketSize
 * Description: Set the max packet size
 * Param:
 *     param - string represention of the value of the throttle.  If it is not
 *     a number the value will be set to 0.
 * Return:
 *     return true if the throttle was set correctly, otherwise false.
 *****************************************************************************/
bool PortAgentConfig::setMaxPacketSize(const string &param) {
    const char* v = param.c_str();
    
    int value = atoi(v);
    
    if(value <= 0) {
        LOG(ERROR) << "Invalid max packet size.  using default " << DEFAULT_PACKET_SIZE;
        m_maxPacketSize = DEFAULT_PACKET_SIZE;
        return false;
    }
    
    if(value > MAX_PACKET_SIZE) {
        LOG(ERROR) << "packet size exceeds maximum value, " << MAX_PACKET_SIZE
                   << " using default " << DEFAULT_PACKET_SIZE;
        m_maxPacketSize = DEFAULT_PACKET_SIZE;
        return false;
    }
    
    LOG(INFO) << "set max packet size to " << value;
    m_maxPacketSize = value;
    return true;
}

/******************************************************************************
 * Method: setLogLevel
 * Description: Change the log level
 * Return:
 *     return true if the log level was set correctly, otherwise false.
 *****************************************************************************/
bool PortAgentConfig::setLogLevel(const string &param) {
    string str = param;
    transform(str.begin(), str.end(),str.begin(), ::toupper);
    
    if(str == "WARN")
        str = "WARNING";
        
    Logger::SetLogLevel(str);
    
    if(Logger::GetError()) {
        return false;
    }
    
    return true;
}

/******************************************************************************
 * Method: setDevice
 * Description: Set the device path
 * Return:
 *     return true if set correctly, otherwise false.
 *****************************************************************************/
bool PortAgentConfig::setDevicePath(const string &param) {
    bool    bReturnCode = true;
    string  devicePath = param;

    if (!param.size() > 0) {
        bReturnCode = false;
    }
    else {
        m_devicePath = devicePath;
    }
    return bReturnCode;
}

/******************************************************************************
 * Method: setBaud
 * Description: Change the baud
 * Return:
 *     return true if set correctly, otherwise false.
 *****************************************************************************/
bool PortAgentConfig::setBaud(const string &param) {
    uint32_t baud = atoi(param.c_str());
    
    if( baud != 1200 && baud != 2400 && baud != 4800 && baud != 9600 && 
       baud != 19200 && baud != 38400 && baud != 57600 && baud != 115200 ) {
        LOG(ERROR) << "Invalid baud rate: " << baud;
        m_baud = 0;
        return false;
    }
    
    m_baud = baud;
    return true;
}

/******************************************************************************
 * Method: setStopbits
 * Description: Change the stopbits
 * Return:
 *     return true if set correctly, otherwise false.
 *****************************************************************************/
bool PortAgentConfig::setStopbits(const string &param) {
    uint32_t bits = atoi(param.c_str());
    
    if( bits != 1  && bits != 2 ) {
        LOG(ERROR) << "Invalid stop bits: " << bits;
        m_stopbits = 1;
        return false;
    }
    
    m_stopbits = bits;
    return true;
}

/******************************************************************************
 * Method: setDatabits
 * Description: Change the data bits
 * Return:
 *     return true if set correctly, otherwise false.
 *****************************************************************************/
bool PortAgentConfig::setDatabits(const string &param) {
    uint32_t bits = atoi(param.c_str());
    
    if( bits != 5 && bits != 6 && bits != 7 && bits != 8 ) {
        LOG(ERROR) << "Invalid data bits: " << bits;
        m_databits = 8;
        return false;
    }
    
    m_databits = bits;
    return true;
}

/******************************************************************************
 * Method: setParity
 * Description: Change the parity
 * Return:
 *     return true if set correctly, otherwise false.
 *****************************************************************************/
bool PortAgentConfig::setParity(const string &param) {
    uint32_t parity = atoi(param.c_str());
    const char *buffer = param.c_str();
    
    if( (buffer[0] != '0' && parity == 0) ||
        (parity != 0 && parity != 1 && parity != 2) ) {
        LOG(ERROR) << "Invalid parity: " << parity;
        m_parity = 0;
        return false;
    }
    
    m_parity = parity;
    return true;
}


/******************************************************************************
 * Method: setFlow
 * Description: Change the flow control
 * Return:
 *     return true if set correctly, otherwise false.
 *****************************************************************************/
bool PortAgentConfig::setFlow(const string &param) {
    uint32_t flow = atoi(param.c_str());
    const char *buffer = param.c_str();
    
    if( (buffer[0] != '0' && flow == 0) ||
        (flow != 0 && flow != 1 && flow != 2) ) {
        LOG(ERROR) << "Invalid flow: " << flow;
        m_flow = 0;
        return false;
    }
    
    m_flow = flow;
    return true;
}

/******************************************************************************
 * Method: setRotationInterval
 * Description: Set data log rotation interval
 * Return:
 *     return true if the type was set correctly, otherwise false for unknown
 *     types. Default to DAILY
 *****************************************************************************/
bool PortAgentConfig::setRotationInterval(const string &param) {
    m_eRotationInterval = DAILY;
    
    if(param == "daily") {
        LOG(INFO) << "data log rotation set to daily";
        m_eRotationInterval = DAILY;
    }
    
    else if(param == "hourly") {
        LOG(INFO) << "data log rotation set to hourly";
        m_eRotationInterval = HOURLY;
    }
    
    else if(param == "quarter_hourly") {
        LOG(INFO) << "data log rotation set to quarter hourly";
        m_eRotationInterval = QUARTER_HOURLY;
    }
    
    else if(param == "minute") {
        LOG(INFO) << "data log rotation set to minute";
        m_eRotationInterval = MINUTE;
    }
    
    else {
        LOG(ERROR) << "unknown log rotation type: " << param;
        return false;
    }
    
    return true;
}

/******************************************************************************
 * Method: setTelnetSnifferPort
 * Description: Set the telnet sniffer port
 * Param:
 *     param - string represention of the value of the port.  If it is not
 *     a number the value will be set to 0.
 * Return:
 *     return true if the port was set correctly, otherwise false.
 *****************************************************************************/
bool PortAgentConfig::setTelnetSnifferPort(const string &param) {
    const char* v = param.c_str();
    
    int value = atoi(v);
    m_telnetSnifferPort = 0;
    
    if(value <= 0 || value > 65535) {
        LOG(ERROR) << "Invalid port specification, setting to 0";
        return false;
    }
    
    LOG(INFO) << "set telnet sniffer port to " << value;
    m_telnetSnifferPort = value;
    return true;
}


/******************************************************************************
 *   PRIVATE METHODS
 ******************************************************************************/
/******************************************************************************
 * Method: setParameter(char option, char *value)
 * Description: Set the object parameter
 * Exceptions:
 *     ParameterRequired()
 * Return:
 *     return true if options are valid.
 *****************************************************************************/
void PortAgentConfig::setParameter(char option, char *value) {
    switch(option) {
        case 'c':
            readConfig(value);
            m_conffile = value; break;
        case 'p':
            m_observatoryCommandPort = atoi(value); break;
        case 's':
            m_noDetatch = true; break;
        case 'h':
            addCommand(CMD_HELP);
            m_help = true;
            break;
        case 'n':
            addCommand(CMD_SHUTDOWN);
            m_version = true;
            break;
        case 'k':
            addCommand(CMD_SHUTDOWN);
            m_kill = true;
            break;
        case 'v':
            m_verbose++;
            Logger::IncreaseLogLevel(1);
            break;
        case 'y':
            if(!value)
                throw ParameterRequired("ppid");
            
            m_ppid = atoi(value);
            break;
        case '?':
            throw ParameterRequired();
    };
}


/******************************************************************************
 * Method: verifyCommandLineParameters()
 * Description: Verify we have all the information we need to start from the
 *              command line.  
 * Exceptions:
 *     ParameterRequired()
 *****************************************************************************/
void PortAgentConfig::verifyCommandLineParameters() {
    string error;
    // If help then no verification needed.
    if(m_help == CMD_HELP || m_version )
        return;

    // Ensure we have an observatory command port because that's is the
    // port agent's unique identifier.
    if(observatoryCommandPort() == 0)
        throw ParameterRequired("observatoryCommandPort");
    
    // Ensure the key directories exist and are writable.
    if(! mkpath(logdir()) ) {
        error = "could not create logdir, " + logdir();
    }
    if(! mkpath(piddir()) ) {
        error = "could not create piddir, " + piddir();
    }
    if(! mkpath(datadir()) ) {
        error = "could not create datadir, " + datadir();
    }
        
    if(error.length())
        throw FileIOException(error);
        
}


/******************************************************************************
 * Method: addCommand()
 * Description: Add a command to the command queue.  First search the queue,
 * if the command already exists, don't add it again.
 ******************************************************************************/
void PortAgentConfig::addCommand(PortAgentCommand command) {
    bool cmdFound = false;
    list<PortAgentCommand>::iterator i = m_commands.begin();
    for(i = m_commands.begin(); i != m_commands.end(); i++)
        if(*i == command)
            cmdFound = true;
    
    if(! cmdFound) {
        LOG(DEBUG) << "Command added: " << command;
        m_commands.push_back(command);
    }
    else {
        LOG(DEBUG) << "Command already in command queue.  not adding again";
    }
}


/******************************************************************************
 * Method: processCommand()
 * Description: Read a command from a string and then process it.  This would
 *              be setting the configuration parameter if needed and adding a
 *              command to the queue if needed.
 * Return: true if successfully parsed.
 ******************************************************************************/
bool PortAgentConfig::processCommand(const string & command) {
    string cmd, param;
    splitCommand(command, cmd, param);
    
    ///////////////////////////
    // First look for commands
    ///////////////////////////
    if( command == "help" )
        addCommand(CMD_HELP);
        
    else if( command == "verbose" )
        Logger::IncreaseLogLevel(1);
        
    else if( command == "save_config" )
        addCommand(CMD_SAVE_CONFIG);
        
    else if( command == "get_config" )
        addCommand(CMD_GET_CONFIG);
        
    else if( command == "get_state" )
        addCommand(CMD_GET_STATE);
        
    else if( command == "ping" )
        addCommand(CMD_PING);
        
    else if( command == "shutdown" )
        addCommand(CMD_SHUTDOWN);
        
    
    ///////////////////////////
    // Check for parameters
    ///////////////////////////
    
    else if(cmd == "break" ) {
        addCommand(CMD_BREAK);
        return setInstrumentBreakDuration(param);
    }

    else if(cmd == "instrument_type") {
        addCommand(CMD_COMM_CONFIG_UPDATE);
        return setInstrumentConnectionType(param);
    }
    
    else if(cmd == "observatory_type") {
        addCommand(CMD_COMM_CONFIG_UPDATE);
        return setObservatoryConnectionType(param);
    }

    else if(cmd == "sentinle") {
        // We pass the entire command string to this incase there are \n or \r
        // embedded in the sentinle string.
        addCommand(CMD_PUBLISHER_CONFIG_UPDATE);
        return setSentinleSequence(command);
    }
    
    else if(cmd == "output_throttle") {
        addCommand(CMD_COMM_CONFIG_UPDATE);
        return setOutputThrottle(param);
    }
    
    else if(cmd == "heartbeat_interval") {
        return setHeartbeatInterval(param);
    }
    
    else if(cmd == "max_packet_size") {
        addCommand(CMD_PUBLISHER_CONFIG_UPDATE);
        return setMaxPacketSize(param);
    }
    
    else if(cmd == "data_port") {
        addCommand(CMD_COMM_CONFIG_UPDATE);
        return setObservatoryDataPort(param);
    }

    // DHE NEW:
    else if(cmd == "add_data_port") {
        addCommand(CMD_COMM_CONFIG_UPDATE);
        return addObservatoryDataPort(param);
    }
    
    else if(cmd == "command_port") {
        addCommand(CMD_COMM_CONFIG_UPDATE);
        return setObservatoryCommandPort(param);
    }
    
    else if(cmd == "instrument_data_port") {
        addCommand(CMD_COMM_CONFIG_UPDATE);
        return setInstrumentDataPort(param);
    }
    
    else if(cmd == "instrument_data_tx_port") {
        addCommand(CMD_COMM_CONFIG_UPDATE);
        return setInstrumentDataTxPort(param);
    }

    else if(cmd == "instrument_data_rx_port") {
        addCommand(CMD_COMM_CONFIG_UPDATE);
        return setInstrumentDataRxPort(param);
    }

    else if(cmd == "instrument_command_port") {
        addCommand(CMD_COMM_CONFIG_UPDATE);
        return setInstrumentCommandPort(param);
    }
    
    else if(cmd == "log_level") {
        return setLogLevel(param);
    }
    
    else if(cmd == "log_dir") {
        m_logdir = param;
        string file = logfile();
        if(file.length())
            Logger::SetLogFile(file);
    }
    
    else if(cmd == "pid_dir") {
        addCommand(CMD_PATH_CONFIG_UPDATE);
        m_piddir = param;
    }
    
    else if(cmd == "data_dir") {
        addCommand(CMD_PATH_CONFIG_UPDATE);
        m_datadir = param;
    }
    
    else if(cmd == "conf_dir") {
        addCommand(CMD_PATH_CONFIG_UPDATE);
        m_confdir = param;
    }
    
    else if(cmd == "log_dir") {
        addCommand(CMD_PATH_CONFIG_UPDATE);
        m_logdir = param;
    }
    
    else if(cmd == "instrument_addr") {
        addCommand(CMD_COMM_CONFIG_UPDATE);
        m_instrumentAddr = param;
    }
    
    else if(cmd == "device_path") {
        m_bDevicePathChanged = true;
        addCommand(CMD_COMM_CONFIG_UPDATE);
        return setDevicePath(param);
    }

    // For baud, stopbits, databits, parity, and flow we set the
    // m_bSerialSettingsChanged flag; if just the serial settings
    // have changed (and not the device_path), we don't want to
    // close the device driver but rather just reinitialize the
    // serial settings.
    else if(cmd == "baud") {
        m_bSerialSettingsChanged = true;
        addCommand(CMD_COMM_CONFIG_UPDATE);
        return setBaud(param);
    }
    
    else if(cmd == "stopbits") {
        m_bSerialSettingsChanged = true;
        addCommand(CMD_COMM_CONFIG_UPDATE);
        return setStopbits(param);
    }
    
    else if(cmd == "databits") {
        m_bSerialSettingsChanged = true;
        addCommand(CMD_COMM_CONFIG_UPDATE);
        return setDatabits(param);
    }
    
    else if(cmd == "parity") {
        m_bSerialSettingsChanged = true;
        addCommand(CMD_COMM_CONFIG_UPDATE);
        return setParity(param);
    }
    
    else if(cmd == "flow") {
        m_bSerialSettingsChanged = true;
        addCommand(CMD_COMM_CONFIG_UPDATE);
        return setFlow(param);
    }
    
    else if(cmd == "rotation_interval") {
        addCommand(CMD_ROTATION_INTERVAL);
        return setRotationInterval(param);
    }
    
    else if(cmd == "telnet_sniffer_port") {
        addCommand(CMD_PUBLISHER_CONFIG_UPDATE);
        return setTelnetSnifferPort(param);
    }
    
    else if(cmd == "telnet_sniffer_prefix") {
        addCommand(CMD_PUBLISHER_CONFIG_UPDATE);
        return setTelnetSnifferPrefix(param);
    }
    
    else if(cmd == "telnet_sniffer_suffix") {
        addCommand(CMD_PUBLISHER_CONFIG_UPDATE);
        return setTelnetSnifferSuffix(param);
    }
    
    // Couldn't parse this command
    else {
        LOG(ERROR) << "Failed to parse command: " << cmd;
        return false;
    }
    
    return true;
        
}

/******************************************************************************
 * Method: splitCommand()
 * Description: Split a command string into a command and parameter.
 * Return: return true if we could successfully parse.
 ******************************************************************************/
bool PortAgentConfig::splitCommand(const string &raw, string & cmdResult, string & paramResult) {
    string cmd, param, dump;
    istringstream iss(raw);
    
    // Read the command
    iss >> cmd;
    
    // Read a parameter if it exists
    iss >> param;
    
    // If there are any trailing tokens it's an error
    iss >> dump;
    if(dump.length()) {
        LOG(ERROR) << "trailing config tokens found in: " << raw;
        return false;
    }
    
    // We made it through the parser, store the results
    chomp(cmd);
    chomp(param);
    cmdResult = cmd;
    paramResult = param;
    return true;
}

ObservatoryDataPorts* ObservatoryDataPorts::m_pInstance = 0;

ObservatoryDataPorts::ObservatoryDataPorts() {

}

/******************************************************************************
 * Method: instance()
 * Description: Return a pointer to the singleton instance of the
 * ObservatoryDataPorts container.  If the singleton does not yet exist, create
 * it.
 * Return: return a pointer to the instance.
 ******************************************************************************/
ObservatoryDataPorts* ObservatoryDataPorts::instance() {

    if (0 == m_pInstance) {
       m_pInstance = new ObservatoryDataPorts();
    }

    return m_pInstance;
}

/******************************************************************************
 * Method: logPorts()
 * Description: Log the ports
 * Return: void
 ******************************************************************************/
void ObservatoryDataPorts::logPorts() {

    list<ObservatoryDataPortEntry_T>::iterator i = m_observatoryDataPorts.begin();
    int j = 0;
    for(i = m_observatoryDataPorts.begin(); i != m_observatoryDataPorts.end(); i++) {
        LOG(DEBUG) << "Data port: " << j << ", " << *i;
        j++;
    }
}

/******************************************************************************
 * Method: addPort(const int port)
 * Description: Add the given port to the container of ports.
 * Return: return true if success, false if not.
 ******************************************************************************/
bool ObservatoryDataPorts::addPort(const int port) {
    bool bRetVal = true;

    LOG(DEBUG) << "ObservatoryDataPorts::addPort: Adding port: " << port;

    // First remove any existing element with the same port value
    m_observatoryDataPorts.remove(port);
    m_observatoryDataPorts.push_back(port);

    return bRetVal;
}

/******************************************************************************
 * Method: getFirstPort
 * Description: Get the first port from the container of ports.
 * Return: value of first port if there is one, otherwize 0
 ******************************************************************************/
const int ObservatoryDataPorts::getFirstPort()
{

   // get the first cam agent
   m_portIt = m_observatoryDataPorts.begin();

   // if the first is the end, there are no ports in the list
   if (m_portIt == m_observatoryDataPorts.end()) {
      return 0;
   }
   else {
      return *m_portIt;
   }
}

/******************************************************************************
 * Method: getNextPort
 * Description: Get the next port from the container of ports.
 * Return: value of next port if there is one, otherwize 0
 ******************************************************************************/
const int ObservatoryDataPorts::getNextPort()
{

   m_portIt++;

   // if the next is the end, there are no more ports in the list
   if (m_portIt == m_observatoryDataPorts.end()) {
      return 0;
   }
   else {
      return *m_portIt;
   }
}


