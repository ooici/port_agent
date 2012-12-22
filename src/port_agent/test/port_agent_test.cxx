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
#include "gtest/gtest.h"
#include "common/util.h"

using namespace logger;
using namespace std;
using namespace port_agent;

const char* TEST_OB_CMD_PORT = "9001";
const char* TEST_OB_DATA_PORT = "9002";
const char* TEST_IN_DATA_PORT = "9003";

const char* RESPONSE_FILE="/tmp/gtest.rsp";
const char* DUMP_FILE="/tmp/gtest.dmp";
const char* CONFIG_FILE="/tmp/gtest.cfg";
const char* CMD_FILE="/tmp/gtest.c,d";
const char* TEST_LOG="/tmp/gtest.log";
const char* FILE_LOG="/tmp/gtest.out";
const char* SERVER_LOG="/tmp/gtest.srv";
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
            
        void startPortAgent() {
            try {
                stringstream cmd;
                cmd << "../port_agent";
                stringstream portStr;
                portStr << TEST_OB_CMD_PORT;

                SpawnProcess process(cmd.str(), 8, "-v", "-v", "-v", "-v", "-v", "-v", "-p",
                portStr.str().c_str()); 

                LOG(INFO) << "Start Port Agent: " << process.cmd_as_string();

                process.run();
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
            
            if(config.length())
                cmd << config;
            else 
                cmd << "instrument_type tcp" << endl
                    << "instrument_data_port " << TEST_IN_DATA_PORT << endl
                    << "instrument_addr localhost" << endl
                    << "data_port " << TEST_OB_DATA_PORT << endl
                    << "command_port " << TEST_OB_CMD_PORT << endl;
                    
            LOG(DEBUG) << "Port agent config: " << cmd.str();
            
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
        
        void sendDriverData(const string &cmd) {
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
            process.run();
            
            while(process.is_running()) {
                LOG(DEBUG) << "Waiting for client process die.";
                sleep(1);
            }
        }
        
        virtual void TearDown() {
            LOG(INFO) << "Tear down test";
        
            while(m_oProcess.is_running()) {
                LOG(DEBUG) << "Waiting for client to die.";
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
                 "5" );

            LOG(INFO) << "Start TCP Echo Server: " << process.cmd_as_string();
            process.set_output_file(SERVER_LOG);
    
            bool result = process.run();
            sleep(1);
        
        m_oProcess = process;
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
    
        protected:
        SpawnProcess m_oProcess;
        SpawnProcess m_oDumpProcess;
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
//TEST_F(PortAgentUnitTest, StartUp) {
    try {
        string response;
        string datafile = getDataFile();
        
        remove_file(RESPONSE_FILE);
        
        startTCPEchoServer();
        startPortAgent();
        configurePortAgent();
        response = commandPortAgent("get status");
        
        startTCPClientDump(atoi(TEST_OB_CMD_PORT), "localhost", RESPONSE_FILE);
        
        sendDriverData("foo");
        
        stopTCPClientDump();
    }
    catch(exception &e) {
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

