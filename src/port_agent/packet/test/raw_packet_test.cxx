#include "raw_packet.h"
#include "common/logger.h"
#include "gtest/gtest.h"
#include "common/util.h"
#include "port_agent/config/port_agent_config.h"

#include <sys/types.h>
#include <unistd.h>
#include <string>

using namespace logger;
using namespace std;
using namespace packet;

class RawPacketTest : public testing::Test {
    protected:
        virtual void SetUp() {
            Logger::SetLogFile("/tmp/gtest.log");
            Logger::SetLogLevel("DEBUG");

            LOG(INFO) << "************************************************";
            LOG(INFO) << "            Raw Packet Test Start Up";
            LOG(INFO) << "************************************************";
        }

        virtual void TearDown() {
            LOG(INFO) << "RawPacketTest TearDown";
            stringstream out;
        }

        ~RawPacketTest() {
            LOG(INFO) << "RawPacketTest dtor";
        }

        void const printRawBytes(stringstream& out, const char* buffer, const size_t numBytes) {
            for (size_t ii = 0; ii < numBytes;ii++)
                out << setfill('0') << setw(2) << hex << uppercase << byteToUnsignedInt(buffer[ii]);
        }

        RawPacket* buildRawPacket(char* rawBuffer, size_t bufferSize) {
            char syncData[] = {0xA3, 0x9D, 0x7A};
            char typeData = 1+(rand() % 7); // Random packet type
            uint16_t rawSize = htons(MAX_PACKET_SIZE);
            Timestamp timeStamp;
            uint64_t rawTimeStamp = timeStamp.asBinary();
            uint32_t payLoadSize = MAX_PACKET_SIZE - HEADER_SIZE;
            char payLoadData[payLoadSize];
            // Populate payload randomly
            for (uint32_t ii = 0; ii < payLoadSize; ii++) {
                payLoadData[ii] = (rand() % 256);
            }
            memcpy(rawBuffer, syncData, 3);
            rawBuffer[3] = typeData;
            memcpy(rawBuffer+4, &rawSize, 2);
            memcpy(rawBuffer+8, &rawTimeStamp, 8);
            memcpy(rawBuffer+16, payLoadData, payLoadSize);
            RawPacket* rawPacket = reinterpret_cast<RawPacket*>(rawBuffer);
            uint16_t rawCheckSum = htons(rawPacket->calculateChecksum(rawPacket));
            memcpy(rawBuffer+6, &rawCheckSum, 2);

            return reinterpret_cast<RawPacket*>(rawBuffer);
        }
};

TEST_F(RawPacketTest, GetterTest) {

    stringstream out;

    // Build raw packet
    char rawPacketData[MAX_PACKET_SIZE];
    RawPacket* rawPacket = buildRawPacket(rawPacketData, MAX_PACKET_SIZE);

    ASSERT_TRUE(rawPacket->validateChecksum());

    Packet basicPacket(rawPacket->getPacketType(), rawPacket->getTimestamp(), rawPacket->getPayload(), rawPacket->getPayloadSize());
    char* basicPacketRaw = basicPacket.packet();

    out << endl << "rawHeader            = ";
    printRawBytes(out, rawPacketData, HEADER_SIZE);
    out << endl << "basicPacketRawHeader = ";
    printRawBytes(out, basicPacketRaw, HEADER_SIZE);

    LOG(INFO) << out.str();

    ASSERT_FALSE(memcmp(rawPacket, basicPacketRaw, MAX_PACKET_SIZE));
}

TEST_F(RawPacketTest, ValidateHeaderTest) {
    // Build raw packet
    char rawPacketData[MAX_PACKET_SIZE];
    RawPacket* rawPacket = buildRawPacket(rawPacketData, MAX_PACKET_SIZE);

    ASSERT_TRUE(rawPacket->validateHeader(MAX_PACKET_SIZE));

    // Invalid sync
    char invalidSync[] = {0x45, 0x89, 0xF4};
    memcpy(rawPacketData, invalidSync, 3);
    ASSERT_EQ(rawPacket->getSync(), 0x4589F4);
    ASSERT_FALSE(rawPacket->validateHeader(MAX_PACKET_SIZE));
    rawPacket = buildRawPacket(rawPacketData, MAX_PACKET_SIZE);
    ASSERT_TRUE(rawPacket->validateHeader(MAX_PACKET_SIZE));

    // Invalid type less than
    rawPacketData[3] = 0;
    ASSERT_EQ(rawPacket->getPacketType(), 0);
    ASSERT_FALSE(rawPacket->validateHeader(MAX_PACKET_SIZE));
    rawPacket = buildRawPacket(rawPacketData, MAX_PACKET_SIZE);
    ASSERT_TRUE(rawPacket->validateHeader(MAX_PACKET_SIZE));

    // Invalid type greater than
    rawPacketData[3] = 8;
    ASSERT_FALSE(rawPacket->validateHeader(MAX_PACKET_SIZE));
    rawPacket = buildRawPacket(rawPacketData, MAX_PACKET_SIZE);
    ASSERT_TRUE(rawPacket->validateHeader(MAX_PACKET_SIZE));

    // Invalid size less than HEADER_SIZE
    char invalidSize1[] = {0x00, 0x02};
    memcpy(rawPacketData+4, &invalidSize1, 2);
    ASSERT_EQ(rawPacket->getPacketSize(), 0x0002);
    ASSERT_FALSE(rawPacket->validateHeader(MAX_PACKET_SIZE));
    rawPacket = buildRawPacket(rawPacketData, MAX_PACKET_SIZE);
    ASSERT_TRUE(rawPacket->validateHeader(MAX_PACKET_SIZE));

    // Invalid size greater than MAX_PACKET_SIZE
    char invalidSize2[] = {0xFF, 0xFF};
    memcpy(rawPacketData+4, &invalidSize2, 2);
    ASSERT_EQ(rawPacket->getPacketSize(), 0xFFFF);
    ASSERT_FALSE(rawPacket->validateHeader(MAX_PACKET_SIZE));
    rawPacket = buildRawPacket(rawPacketData, MAX_PACKET_SIZE);
    ASSERT_TRUE(rawPacket->validateHeader(MAX_PACKET_SIZE));
}

TEST_F(RawPacketTest, ChecksumTest) {

    // Build raw packet
    char rawPacketData[MAX_PACKET_SIZE];
    RawPacket* rawPacket = buildRawPacket(rawPacketData, MAX_PACKET_SIZE);

    ASSERT_TRUE(rawPacket->validateChecksum());

    uint16_t rawCheckSum = rawPacket->getChecksum();
    uint16_t incorrectCheckSum = ++rawCheckSum;
    memcpy(rawPacketData+6, &incorrectCheckSum, 2);

    ASSERT_FALSE(rawPacket->validateChecksum());
}
