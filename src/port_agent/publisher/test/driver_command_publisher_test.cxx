#include "common/logger.h"
#include "common/util.h"
#include "port_agent/packet/packet.h"
#include "gtest/gtest.h"
#include "publisher_test.h"
#include "driver_command_publisher.h"
#include "network/tcp_comm_socket.h"
#include "network/tcp_comm_listener.h"


#include <sstream>
#include <string>
#include <string.h>

using namespace std;
using namespace packet;
using namespace logger;
using namespace publisher;
using namespace network;

#define DATAFILE "/tmp/data.log"

class DriverCommandPublisherTest : public FilePointerPublisherTest {
    
    protected:
        virtual void SetUp() {
            Logger::SetLogFile("/tmp/gtest.log");
            Logger::SetLogLevel("MESG");
            
            LOG(INFO) << "************************************************";
            LOG(INFO) << "   DriverCommandPublisherTest Test Start Up";
            LOG(INFO) << "************************************************";

            datafile = DATAFILE;
        }
};

/* Test Basic Creation and ASCII out */
TEST_F(DriverCommandPublisherTest, SingleAsciiOut) {
	DriverCommandPublisher publisher;
	EXPECT_TRUE(testPublish(publisher, DATA_FROM_DRIVER, true));
	EXPECT_TRUE(testPublish(publisher, DATA_FROM_INSTRUMENT, true));
	//EXPECT_TRUE(testPublish(publisher, PORT_AGENT_COMMAND, true));
	EXPECT_TRUE(testPublish(publisher, PORT_AGENT_STATUS, true));
	EXPECT_TRUE(testPublish(publisher, PORT_AGENT_FAULT, true));
	EXPECT_TRUE(testPublish(publisher, INSTRUMENT_COMMAND, true));
}

/* Test Single binary packet out out */
TEST_F(DriverCommandPublisherTest, SingleBinaryOut) {
	DriverCommandPublisher publisher;

	// It shouldn't matter if the binary flag is set, but let's test anyway.
	EXPECT_TRUE(testPublish(publisher, DATA_FROM_DRIVER, false));
	EXPECT_TRUE(testPublish(publisher, DATA_FROM_INSTRUMENT, false));
	//EXPECT_TRUE(testPublish(publisher, PORT_AGENT_COMMAND, false));
	EXPECT_TRUE(testPublish(publisher, PORT_AGENT_STATUS, false));
	EXPECT_TRUE(testPublish(publisher, PORT_AGENT_FAULT, false));
	EXPECT_TRUE(testPublish(publisher, INSTRUMENT_COMMAND, false));
}

/* Test publication failures */
TEST_F(DriverCommandPublisherTest, DISABLED_FailureNoFile) {
	DriverCommandPublisher publisher;
	EXPECT_TRUE(testPublishFailure(publisher, INSTRUMENT_COMMAND));
}

/* Test with a tcp comm socket */
TEST_F(DriverCommandPublisherTest, DISABLED_CommSocketWrite) {
    try {
	    TCPCommSocket socket;
        socket.setHostname("localhost");
        socket.setPort(4001);
	
        DriverCommandPublisher publisher((CommBase*)&socket);
	
        EXPECT_TRUE(testPublishCommSocket(publisher, 4001, DATA_FROM_DRIVER));
	}
	catch(OOIException &e) {
		string err = e.what();
		LOG(ERROR) << "EXCEPTION: " << err;
		ASSERT_FALSE(true);
	}
}

// Test equality operator
TEST_F(DriverCommandPublisherTest, TCPCommSocketEqualityOperator) {
	try {
    	DriverCommandPublisher leftPublisher, rightPublisher;
    	TCPCommSocket leftSocket, rightSocket;
    	
    	EXPECT_TRUE(leftPublisher == leftPublisher);
    	EXPECT_TRUE(leftPublisher == rightPublisher);
    	
    	// Test with sockets
    	leftSocket.setHostname("localhost");
        leftSocket.setPort(4001);
    	
	    rightSocket.setHostname("localhost");
        rightSocket.setPort(4001);
    	    
    	EXPECT_TRUE(leftSocket == rightSocket);
    	leftPublisher.setCommObject(&leftSocket);
    	EXPECT_TRUE(leftPublisher.commSocket());
    	
	    EXPECT_FALSE(leftPublisher == rightPublisher);
	
	    rightPublisher.setCommObject(&rightSocket);
	    EXPECT_TRUE(rightPublisher.commSocket());
	    EXPECT_TRUE(leftPublisher == rightPublisher);
        
		rightSocket.setPort(4002);
	    rightPublisher.setCommObject(&rightSocket);
	    EXPECT_FALSE(leftPublisher == rightPublisher);
	}
	catch(OOIException &e) {
		string err = e.what();
		LOG(ERROR) << "EXCEPTION: " << err;
		ASSERT_FALSE(true);
	}
}

// Test equality operator
TEST_F(DriverCommandPublisherTest, TCPListenerEqualityOperator) {
	try {
    	DriverCommandPublisher leftPublisher, rightPublisher;
    	TCPCommListener leftSocket;
		TCPCommSocket rightSocket;
    	
    	EXPECT_TRUE(leftPublisher == leftPublisher);
    	EXPECT_TRUE(leftPublisher == rightPublisher);
    	
    	leftPublisher.setCommObject(&leftSocket);
	    rightPublisher.setCommObject(&rightSocket);
    	
	    EXPECT_FALSE(leftPublisher == rightPublisher);
	}
	catch(OOIException &e) {
		string err = e.what();
		LOG(ERROR) << "EXCEPTION: " << err;
		ASSERT_FALSE(true);
	}
}

