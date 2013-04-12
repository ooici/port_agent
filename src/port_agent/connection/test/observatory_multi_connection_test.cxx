#include "common/exception.h"
#include "common/logger.h"
#include "common/util.h"
#include "port_agent/connection/observatory_multi_connection.h"
#include "gtest/gtest.h"

#include <sstream>
#include <string>
#include <string.h>

using namespace std;
using namespace logger;
using namespace port_agent;

#define TEST_DATA_PORT_01 6001
#define TEST_DATA_PORT_02 6002
#define TEST_COMMAND_PORT 6000

class ObservatoryMultiConnectionTest : public testing::Test {
    
    protected:
        virtual void SetUp() {
            Logger::SetLogFile("/tmp/gtest.log");
            Logger::SetLogLevel("MESG");
            
            LOG(INFO) << "************************************************";
            LOG(INFO) << "     Observatory Multi Connection Test Start Up";
            LOG(INFO) << "************************************************";
        }
};

/* Test Normal Observatory Connection */
TEST_F(ObservatoryMultiConnectionTest, DISABLED_NormalConnection) {
	CommBase *dataConnection;
	CommBase *commandConnection;
	
    try {
        ObservatoryMultiConnection connection;
        Connection *pConnection = &connection;
    
        EXPECT_EQ(pConnection->connectionType(), PACONN_OBSERVATORY_MULTI);
    
        connection.setCommandPort(6000);
    
        connection.initialize();
    
        EXPECT_TRUE(connection.dataInitialized());
        EXPECT_TRUE(connection.commandInitialized());
    
        EXPECT_FALSE(connection.dataConnected());
        EXPECT_FALSE(connection.commandConnected());
    
        ASSERT_TRUE(connection.dataConnectionObject());
        ASSERT_TRUE(connection.commandConnectionObject());
        
        EXPECT_EQ(connection.commandConnectionObject()->getListenPort(), TEST_COMMAND_PORT);
        
        EXPECT_NE(connection.dataConnectionObject()->getListenPort(), TEST_DATA_PORT_01);
        connection.setDataPort(TEST_DATA_PORT_01);
        EXPECT_EQ(connection.dataConnectionObject()->getListenPort(), TEST_DATA_PORT_01);
    }
    catch(OOIException &e) {
		string err = e.what();
		LOG(ERROR) << "EXCEPTION: " << err;
		ASSERT_FALSE(true);
	}
}

