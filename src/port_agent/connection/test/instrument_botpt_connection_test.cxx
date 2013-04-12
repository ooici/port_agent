#include "common/exception.h"
#include "common/logger.h"
#include "common/util.h"
#include "port_agent/connection/instrument_botpt_connection.h"
#include "gtest/gtest.h"

#include <sstream>
#include <string>
#include <string.h>

using namespace std;
using namespace logger;
using namespace port_agent;

#define TEST_DATA_TX_PORT "7001"
#define TEST_DATA_RX_PORT "7002"
#define TEST_DATA_HOST "127.0.0.1"

class InstrumentBOTPTConnectionTest : public testing::Test {
    
    protected:
        virtual void SetUp() {
            Logger::SetLogFile("/tmp/gtest.log");
            Logger::SetLogLevel("MESG");
            
            LOG(INFO) << "************************************************";
            LOG(INFO) << "    Instrument BOTPT Connection Test Start Up";
            LOG(INFO) << "************************************************";
        }
};

/* Test Normal Instrument TCP Connection */
TEST_F(InstrumentBOTPTConnectionTest, NormalConnection) {
    try {
        InstrumentBOTPTConnection connection;
        Connection *pConnection = &connection;
    
        EXPECT_FALSE(connection.dataConfigured());
        EXPECT_FALSE(connection.commandConfigured());
    
        connection.setDataTxPort(atoi(TEST_DATA_TX_PORT));
        EXPECT_FALSE(connection.dataConfigured());
        connection.setDataRxPort(atoi(TEST_DATA_RX_PORT));
        EXPECT_FALSE(connection.dataConfigured());
        connection.setDataHost(TEST_DATA_HOST);
        EXPECT_TRUE(connection.dataConfigured());
    
        EXPECT_EQ(pConnection->connectionType(), PACONN_INSTRUMENT_BOTPT);
    
        connection.initialize();
    
        EXPECT_TRUE(connection.dataInitialized());
        EXPECT_FALSE(connection.commandInitialized());
    
        EXPECT_FALSE(connection.dataConnected());
        EXPECT_FALSE(connection.commandConnected());
    
        ASSERT_TRUE(connection.dataConnectionObject());
        ASSERT_FALSE(connection.commandConnectionObject());
    }
    catch(OOIException &e) {
		string err = e.what();
		LOG(ERROR) << "EXCEPTION: " << err;
		ASSERT_FALSE(false);
	}
}

