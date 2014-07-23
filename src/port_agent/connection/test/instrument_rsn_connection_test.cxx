#include "common/exception.h"
#include "common/logger.h"
#include "common/util.h"
#include "port_agent/connection/instrument_rsn_connection.h"
#include "gtest/gtest.h"

#include <sstream>
#include <string>
#include <string.h>

using namespace std;
using namespace logger;
using namespace port_agent;

#define TEST_DATA_PORT "7001"
#define TEST_DATA_HOST "127.0.0.1"

class InstrumentRSNConnectionTest : public testing::Test {

    protected:
        virtual void SetUp() {
            Logger::SetLogFile("/tmp/gtest.log");
            Logger::SetLogLevel("MESG");

            LOG(INFO) << "************************************************";
            LOG(INFO) << "    Instrument RSN Connection Test Start Up";
            LOG(INFO) << "************************************************";
        }
};

/* Test Normal Instrument RSN Connection */
TEST_F(InstrumentRSNConnectionTest, NormalConnection) {
    try {
        InstrumentRSNConnection connection;
        Connection *pConnection = &connection;

        EXPECT_FALSE(connection.dataConfigured());
        EXPECT_FALSE(connection.commandConfigured());

        connection.setDataPort(atoi(TEST_DATA_PORT));
        EXPECT_FALSE(connection.dataConfigured());
        connection.setDataHost(TEST_DATA_HOST);
        EXPECT_TRUE(connection.dataConfigured());

        EXPECT_EQ(pConnection->connectionType(), PACONN_INSTRUMENT_RSN);

        connection.initialize();

        EXPECT_TRUE(connection.dataInitialized());
        EXPECT_TRUE(connection.commandInitialized());

        EXPECT_FALSE(connection.dataConnected());
        EXPECT_FALSE(connection.commandConnected());

        ASSERT_TRUE(connection.dataConnectionObject());
        ASSERT_TRUE(connection.commandConnectionObject());
    }
    catch(OOIException &e) {
		string err = e.what();
		LOG(ERROR) << "EXCEPTION: " << err;
		ASSERT_FALSE(false);
	}
}

