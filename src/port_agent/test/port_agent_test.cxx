/*******************************************************************************
 * Filename: port_agent_test.h
 * Author: Bill French (wfrench@ucsd.edu)
 * License: Apache 2.0
 *
 * Unit tests for the port agent class.  Integration tests are in a different
 * file.
 *
 ******************************************************************************/

#include "common/exception.h"
#include "common/logger.h"
#include "common/spawn_process.h"
#include "port_agent/port_agent.h"
#include "port_agent/packet/raw_packet.h"
#include "gtest/gtest.h"
#include "common/util.h"
#include <unistd.h>

using namespace logger;
using namespace std;
using namespace port_agent;

const char* TEST_OB_CMD_PORT = "9001";
const char* TEST_OB_DATA_PORT = "9002";
const char* TEST_IN_DATA_PORT = "9003";
const char* TEST_IN_CMD_PORT = "9004";

const char* RESPONSE_FILE="/tmp/gtest.rsp";
const char* DRIVER_RESPONSE_FILE="/tmp/gtest.drsp";
const char* DUMP_FILE="/tmp/gtest.dmp";
const char* CONFIG_FILE="/tmp/gtest.cfg";
const char* CMD_FILE="/tmp/gtest.cmd";
const char* TEST_LOG="/tmp/gtest.log";
const char* FILE_LOG="/tmp/gtest.out";
const char* SERVER_LOG="/tmp/gtest.srv";
const char* RSN_SERVER_LOG="/tmp/gtest.rsn";
const char* PORT_AGENT_LOGBASE="/tmp/port_agent";

class PortAgentUnitTest : public testing::Test {
    protected:
        virtual void SetUp() {
            Logger::SetLogFile(TEST_LOG);
            Logger::SetLogLevel("DEBUG3");
            
            LOG(INFO) << "************************************************";
            LOG(INFO) << "         Port Agent Unit Test Start Up";
            LOG(INFO) << "************************************************";
            
            LOG(INFO) << "Port agent log file: " << portAgentLog();
            
            remove_file(FILE_LOG);
            
            stopPortAgent();
        }
        
        const string portAgentLog() {
            stringstream filename;
            filename << PORT_AGENT_LOGBASE << "_" << TEST_OB_CMD_PORT << ".log";
            return filename.str();
        }
        
        void stopPortAgent() {
            try {
                stringstream cmd;
                cmd << "../port_agent";
                stringstream portStr;
                portStr << TEST_OB_CMD_PORT;

                SpawnProcess process(cmd.str(), 3, "-p", portStr.str().c_str(), "-k");
                process.run();
            }
            catch(exception &e) {
                string err = e.what();
                LOG(ERROR) << "Exception: " << err;
                EXPECT_FALSE(true);
            }
        }

        void startPortAgent(const string &config_file = "") {
            try {
                stringstream cmd;
                cmd << "../port_agent";

                if (config_file.length()) {
                    LOG(INFO) << "Start Port Agent With Config: " << config_file.c_str();
                    SpawnProcess process(cmd.str(), 8, "-v", "-v", "-v", "-v", "-v", "-v", "-c", config_file.c_str());
                    process.run();
                    LOG(INFO) << "Start Port Agent: " << process.cmd_as_string();
                } else {
                    stringstream portStr;
                    portStr << TEST_OB_CMD_PORT;
                    SpawnProcess process(cmd.str(), 8, "-v", "-v", "-v", "-v", "-v", "-v", "-p",
                    portStr.str().c_str());
                    process.run();
                    LOG(INFO) << "Start Port Agent: " << process.cmd_as_string();
                }
                sleep(1);
            }
            catch(exception &e) {
                string err = e.what();
                LOG(ERROR) << "Exception: " << err;
                EXPECT_FALSE(true);
            }
        }
    
        void writeConfig(const string &filename, const string &config = "") {
            stringstream cmd;
            
            LOG(INFO) << "Port agent config length: " << config.length();

            if(config.length())
                cmd << config;
            else 
                cmd << "instrument_type tcp" << endl
                    << "instrument_data_port " << TEST_IN_DATA_PORT << endl
                    << "instrument_addr localhost" << endl
                    << "data_port " << TEST_OB_DATA_PORT << endl
                    << "command_port " << TEST_OB_CMD_PORT << endl
                    << "log_level mesg " << endl;
                    
            LOG(INFO) << "Port agent config: " << endl << cmd.str();
            
            create_file(filename.c_str(), cmd.str().c_str());
        }
        
        void configurePortAgent(const string &config = "") {
            stringstream shell;
            
            writeConfig(CONFIG_FILE, config);
            
            shell << TOOLSDIR << "/tcp_client_write.py";
            
            LOG(DEBUG) << "Run process: " << shell.str();
            
            SpawnProcess process(shell.str(), 4, "-p", TEST_OB_CMD_PORT,
                                 "-f", CONFIG_FILE);
            process.set_output_file(FILE_LOG);
            process.run();
            
            while(process.is_running()) {
                LOG(DEBUG) << "Waiting for client process die.";
                sleep(1);
            }
            
			LOG(DEBUG) << "Configure command output: " << read_file(FILE_LOG);
        }
        
        string commandPortAgent(const string &cmd = "") {
            stringstream shell;
            string response;
            
            LOG(DEBUG) << "Issue port agent command: " << cmd;
            remove_file(RESPONSE_FILE);
            
            create_file(CMD_FILE, cmd.c_str());
            shell << TOOLSDIR << "/tcp_client_write.py";
            
            LOG(DEBUG) << "Run process: " << shell.str();
            
            SpawnProcess process(shell.str(), 8, "-p", TEST_OB_CMD_PORT,
                                 "-f", CMD_FILE, "-r", RESPONSE_FILE,
                                 "-t", "3" );
            
            process.set_output_file(FILE_LOG);
            process.run();
            
            while(process.is_running()) {
                LOG(DEBUG) << "Waiting for client process die.";
                sleep(1);
            }
            
            LOG(DEBUG2) << "process response: " << response;
            response = read_file(RESPONSE_FILE);
            return response;
        }
        
        bool sendDriverData(const string &cmd) {
            stringstream shell;
            string response;

            remove_file(CMD_FILE);

            LOG(DEBUG) << "Send driver data on port " << TEST_OB_DATA_PORT << ": " << cmd;

            create_file(CMD_FILE, cmd.c_str());
            shell << TOOLSDIR << "/tcp_client_write.py";

            LOG(DEBUG) << "Run process: " << shell.str();

            SpawnProcess process(shell.str(), 4, "-p", TEST_OB_DATA_PORT,
                                 "-f", CMD_FILE);

            process.set_output_file(FILE_LOG);
            bool result = process.run();

            while(process.is_running()) {
                LOG(DEBUG) << "Waiting for client process die.";
                sleep(1);
            }

			return result;
        }
        
        bool sendDriverCommand(const string &cmd) {
        	stringstream shell;
        	string response;

        	remove_file(CMD_FILE);

        	LOG(DEBUG) << "Send driver data on port " << TEST_OB_CMD_PORT << ": " << cmd;

        	create_file(CMD_FILE, cmd.c_str());
        	shell << TOOLSDIR << "/tcp_client_write.py";

        	LOG(DEBUG) << "Run process: " << shell.str();

        	SpawnProcess process(shell.str(), 4, "-p", TEST_OB_CMD_PORT,
        			"-f", CMD_FILE);

        	process.set_output_file(FILE_LOG);
        	bool result = process.run();

        	while(process.is_running()) {
        		LOG(DEBUG) << "Waiting for client process die.";
        		sleep(1);
        	}

        	return result;
        }

        bool sendDriverDataWithResponse(const string &cmd) {
            stringstream shell;
            string response;

            remove_file(CMD_FILE);
            remove_file(DRIVER_RESPONSE_FILE);

            LOG(DEBUG) << "Send driver data on port " << TEST_OB_DATA_PORT << ": " << cmd;

            create_file(CMD_FILE, cmd.c_str());
            shell << TOOLSDIR << "/tcp_client_write.py";

            LOG(DEBUG) << "Run process: " << shell.str();

            SpawnProcess process(shell.str(), 8, "-p", TEST_OB_DATA_PORT,
                                 "-f", CMD_FILE, "-r", DRIVER_RESPONSE_FILE, "-t", "10");

            process.set_output_file(FILE_LOG);
            bool result = process.run();

            while(process.is_running()) {
                LOG(DEBUG) << "Waiting for client process die.";
                sleep(1);
            }

            return result;
        }

        virtual void TearDown() {
            LOG(INFO) << "Tear down test";
        
            while(m_oProcess.is_running()) {
                LOG(DEBUG) << "Waiting for client to die.";
                sleep(1);
            }

            while(m_rsnServerProcess.is_running()) {
                LOG(DEBUG) << "Waiting for RSN server to die.";
                sleep(1);
            }

            LOG(DEBUG) << "echo client process complete.";
            stopPortAgent();
        }
    
        ~PortAgentUnitTest() {
            LOG(INFO) << "CommonTest dtor";
        }

        void startTCPEchoServer() {
            stringstream cmd;
            cmd << TOOLSDIR << "/tcp_server_echo.py";
            //cmd << TOOLSDIR << "/testme.py";
            stringstream portStr;
            portStr << TEST_IN_DATA_PORT;

            /***
            SpawnProcess process(cmd.str());
             ***/

            SpawnProcess process(cmd.str(), 5, "-s", "-p",
                 portStr.str().c_str(), "-t",
                 "30");

            LOG(INFO) << "Start TCP Echo Server: " << process.cmd_as_string();
            process.set_output_file(SERVER_LOG);
    
            bool result = process.run();
            sleep(1);
        
            m_oProcess = process;
        }
            
        void startRSNEchoServer() {
            stringstream cmd;
            cmd << TOOLSDIR << "/rsn_server_echo.py";

            SpawnProcess process(cmd.str(), 5, "-s", "-p",
                 TEST_IN_DATA_PORT, "-t",
                 "5");

            LOG(INFO) << "Start RSN Echo Server: " << process.cmd_as_string();
            process.set_output_file(RSN_SERVER_LOG);

            bool result = process.run();
            sleep(1);

            m_rsnServerProcess = process;
        }

        void startRSNEchoServerNoHeader() {
            stringstream cmd;
            cmd << TOOLSDIR << "/rsn_server_echo.py";

            SpawnProcess process(cmd.str(), 6, "-s", "-p",
                 TEST_IN_DATA_PORT, "-t",
                 "5", "-n");

            LOG(INFO) << "Start RSN Echo Server: " << process.cmd_as_string();
            process.set_output_file(RSN_SERVER_LOG);

            bool result = process.run();
            sleep(1);

            m_rsnServerProcess = process;
        }

        void stopRSNEchoServer() {

        }

        void stopTCPClientDump() {
            m_oDumpProcess;
            while(m_oDumpProcess.is_running()) {
                LOG(DEBUG) << "Waiting for process die.";
                sleep(1);
            }
            
            LOG(DEBUG) << "---->>> DHE: this is so weird.";
            LOG(DEBUG) << "Dump output: " << read_file(DUMP_FILE);
        }
        
        void startTCPClientDump(uint16_t port, const string &hostname, const string &file, uint16_t timeout = 10) {
            stringstream cmd;
            
            cmd << TOOLSDIR << "/tcp_client_dump.py";
            stringstream portStr;
            portStr << port;

            stringstream timeoutStr;
            timeoutStr << timeout;
            
            remove_file(DUMP_FILE);

            SpawnProcess process(cmd.str(), 8,
                                    "-t", timeoutStr.str().c_str(),
                                    "-n", hostname.c_str(), 
                                    "-f", file.c_str(),
                                    "-p", portStr.str().c_str()
            );

            LOG(INFO) << "Start TCP Dump Client: " << process.cmd_as_string();
            process.set_output_file(DUMP_FILE);
    
            LOG(DEBUG) << "Program output: " << DUMP_FILE;
            LOG(DEBUG) << "Response file: " << file;
            
            bool result = process.run();
            
            m_oDumpProcess = process;
        }
    
        string getDataFile() {
            stringstream file;
            file << "/var/ooi/port_agent/port_agent_" << TEST_OB_CMD_PORT << "."
                 << ".data";
        
            return file.str();
        }

        size_t readDataFile(const char* fileName, char* data) {

            LOG(DEBUG) << "Read data file: " << fileName;

            size_t length = 0;
            std::ifstream is (fileName, std::ifstream::binary);
            if (is) {
              // get length of file:
              is.seekg (0, is.end);
              length = is.tellg();
              is.seekg (0, is.beg);
              is.read (data, length);

              if (is) {
                  LOG(DEBUG) << "All characters read successfully.";
              } else {
                  LOG(ERROR) << "Only " << is.gcount() << " could be read";
                  length = 0;
              }
              is.close();
            }
            else
            {
                LOG(ERROR) << fileName << " could not be read.";
            }

            return length;
        }

        void const printRawBytes(stringstream& out, const char* buffer, const size_t numBytes) {
            for (size_t ii = 0; ii < numBytes;ii++)
                out << setfill('0') << setw(2) << hex << uppercase << byteToUnsignedInt(buffer[ii]);
        }

        protected:
        SpawnProcess m_oProcess;
        SpawnProcess m_oDumpProcess;
        SpawnProcess m_rsnServerProcess;
};


/////////////////////////////////////////////////////////////////////////////////
// Integration Tests
/////////////////////////////////////////////////////////////////////////////////

/* Test Startup */
TEST_F(PortAgentUnitTest, DISABLED_StartUpWithConfigFile) {
    try {
        string response;
        string datafile = getDataFile();
        
        remove_file(RESPONSE_FILE);
        remove_file(CONFIG_FILE);
        
        writeConfig(CONFIG_FILE);
        
        startPortAgent();
    }
    catch(exception &e) {
        string err = e.what();
        LOG(ERROR) << "Exception: " << err;
        EXPECT_TRUE(false);
    }
}

/* Test Startup */
TEST_F(PortAgentUnitTest, DISABLED_StartUp) {
    try {
        string response;
        string datafile = getDataFile();
        
        remove_file(RESPONSE_FILE);
        
        //startTCPEchoServer();
		
		LOG(ERROR) << "Start port agent";
        startPortAgent();
        configurePortAgent();

        //response = commandPortAgent("get status");

        //startTCPClientDump(atoi(TEST_OB_CMD_PORT), "localhost", RESPONSE_FILE);
        //EXPECT_TRUE(sendDriverData("foo"));
        //stopTCPClientDump();

		// Reconnect and send more data
        //startTCPClientDump(atoi(TEST_OB_CMD_PORT), "localhost", RESPONSE_FILE);
        //EXPECT_TRUE(sendDriverData("foo"));
        //stopTCPClientDump();
    }
    catch(exception &e) {
        string err = e.what();
        LOG(ERROR) << "Exception: " << err;
        EXPECT_TRUE(false);
    }
}

TEST_F(PortAgentUnitTest, DISABLED_RSN_PortAgent) {
    try {

        remove_file(CONFIG_FILE);

        startRSNEchoServer();
        stringstream config;

        config << "instrument_type rsn" << endl
               << "instrument_data_port " << TEST_IN_DATA_PORT << endl
               << "instrument_addr localhost" << endl
               << "data_port " << TEST_OB_DATA_PORT << endl
               << "instrument_command_port " << TEST_IN_CMD_PORT << endl
               << "command_port " << TEST_OB_CMD_PORT << endl
               << "log_level mesg ";

        writeConfig(CONFIG_FILE, config.str());

        startPortAgent(CONFIG_FILE);

        sleep(1);

        char* driverData = "This is a test.";
        size_t driverDataSize = 15;
        sendDriverDataWithResponse(driverData);

        sleep(15);

        char data[MAX_PACKET_SIZE];
        size_t numBytes = readDataFile(DRIVER_RESPONSE_FILE, data);
        LOG(DEBUG) << "Num bytes: " << numBytes;
        ASSERT_NE(numBytes, 0);

        stringstream out;
        printRawBytes(out, data, numBytes);
        LOG(DEBUG) << "Data File: " << out.str().c_str();

        RawPacket* rawPacket = reinterpret_cast<RawPacket*>(data);

        ASSERT_EQ(rawPacket->getPacketType(), DATA_FROM_INSTRUMENT);
        ASSERT_EQ(rawPacket->getPayloadSize(), driverDataSize);
        ASSERT_EQ(rawPacket->getPacketSize(), driverDataSize + HEADER_SIZE);
        ASSERT_FALSE(memcmp(rawPacket->getPayload(), driverData, driverDataSize));

    } catch(exception &e) {
        string err = e.what();
        LOG(ERROR) << "Exception: " << err;
        EXPECT_TRUE(false);
    }
}

TEST_F(PortAgentUnitTest, DISABLED_RSN_PortAgentFault) {
    try {

        remove_file(CONFIG_FILE);

        startRSNEchoServerNoHeader();
        stringstream config;

        config << "instrument_type rsn" << endl
               << "instrument_data_port " << TEST_IN_DATA_PORT << endl
               << "instrument_addr localhost" << endl
               << "data_port " << TEST_OB_DATA_PORT << endl
               << "instrument_command_port " << TEST_IN_CMD_PORT << endl
               << "command_port " << TEST_OB_CMD_PORT << endl
               << "log_level mesg ";

        writeConfig(CONFIG_FILE, config.str());

        startPortAgent(CONFIG_FILE);

        sleep(1);

        char* driverData = "This is invalid because no port agent header was received from the RSN digi.";
        size_t driverDataSize = 76;
        sendDriverDataWithResponse(driverData);

        sleep(15);

        char data[MAX_PACKET_SIZE];
        size_t numBytes = readDataFile(DRIVER_RESPONSE_FILE, data);
        LOG(DEBUG) << "Num bytes: " << numBytes;
        ASSERT_NE(numBytes, 0);

        stringstream out;
        printRawBytes(out, data, numBytes);
        LOG(DEBUG) << "Data File: " << out.str().c_str();

        RawPacket* rawPacket = reinterpret_cast<RawPacket*>(data);

        ASSERT_EQ(rawPacket->getPacketType(), PORT_AGENT_FAULT);
        ASSERT_EQ(rawPacket->getPayloadSize(), driverDataSize);
        ASSERT_EQ(rawPacket->getPacketSize(), driverDataSize + HEADER_SIZE);
        ASSERT_FALSE(memcmp(rawPacket->getPayload(), driverData, driverDataSize));

    } catch(exception &e) {
        string err = e.what();
        LOG(ERROR) << "Exception: " << err;
        EXPECT_TRUE(false);
    }
}


TEST_F(PortAgentUnitTest, DISABLED_RSN_PortAgentDigiIntegration) {
    try {

        string response;
        string datafile = getDataFile();

        remove_file(RESPONSE_FILE);
        remove_file(CONFIG_FILE);

        stringstream config;

        config << "instrument_type rsn" << endl
               << "instrument_data_port " << 2101 << endl
               << "instrument_addr 192.168.1.20" << endl
               << "data_port " << TEST_OB_DATA_PORT << endl
               << "instrument_command_port " << 2102 << endl
               << "command_port " << TEST_OB_CMD_PORT << endl
               << "log_level mesg " << endl;

        writeConfig(CONFIG_FILE, config.str());

        startPortAgent(CONFIG_FILE);

        sleep(5);

        sendDriverData("This is a test.");

        sleep(5);

    } catch(exception &e) {
        string err = e.what();
        LOG(ERROR) << "Exception: " << err;
        EXPECT_TRUE(false);
    }
}

/* Test startup sequence and failures */
// Successful start should end in the unconfigured state

/* Test TCP port agent */
/* Walk through port agent states */
// Make sure we try disonnect and make config changes to transition to unconfigured

/* This test Initializes an RSN Instrument connection, sends data to  get a response
 * from the instrument, and then sends a break command.
 */
TEST_F(PortAgentUnitTest, RSN_PortAgentDigiIntegration) {
    try {

        string response;
        string datafile = getDataFile();

        remove_file(RESPONSE_FILE);
        remove_file(CONFIG_FILE);

        stringstream config;

        config << "instrument_type rsn" << endl
               << "instrument_data_port " << 2101 << endl
               << "instrument_addr 192.168.1.20" << endl
               << "data_port " << TEST_OB_DATA_PORT << endl
               << "instrument_command_port " << 2102 << endl
               << "command_port " << TEST_OB_CMD_PORT << endl
               << "log_level mesg " << endl;

        writeConfig(CONFIG_FILE, config.str());

        startPortAgent(CONFIG_FILE);

        sleep(5);

        char* driverData = "Test with timestamp.";
	    size_t driverDataSize = 15;
	    sendDriverDataWithResponse(driverData);

	    sleep(5);

        sendDriverCommand("break 300");

        sleep(5);

    } catch(exception &e) {
        string err = e.what();
        LOG(ERROR) << "Exception: " << err;
        EXPECT_TRUE(false);
    }
}
