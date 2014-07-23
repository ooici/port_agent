#include "raw_packet_data_buffer.h"
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

class PacketDataBuffer : public testing::Test {

    protected:
        virtual void SetUp() {
            Logger::SetLogFile("/tmp/gtest.log");
            Logger::SetLogLevel("DEBUG");

            LOG(INFO) << "************************************************";
            LOG(INFO) << "            Packet Data Buffer Test Start Up";
            LOG(INFO) << "************************************************";
        }

        virtual void TearDown() {
            LOG(INFO) << "PacketDataBuffer TearDown";
            stringstream out;
        }

        ~PacketDataBuffer() {
            LOG(INFO) << "PacketDataBuffer dtor";
        }

        void const printRawBytes(stringstream& out, const char* buffer, const size_t numBytes) {
            for (size_t ii = 0; ii < numBytes;ii++)
                out << setfill('0') << setw(2) << hex << uppercase << byteToUnsignedInt(buffer[ii]);
        }

        // Generate a random raw packet data
        size_t buildRawPacket(char* rawBuffer) {
            char syncData[] = {0xA3, 0x9D, 0x7A};
            char typeData = 1+(rand() % 7); // Random packet type
            size_t payLoadSize = rand() % (MAX_PACKET_SIZE - HEADER_SIZE);
            uint16_t rawSize = htons(payLoadSize + HEADER_SIZE);
            Timestamp timeStamp;
            uint64_t rawTimeStamp = timeStamp.asBinary();
            char payLoadData[payLoadSize];
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
            return payLoadSize + HEADER_SIZE;
        }
};

TEST_F(PacketDataBuffer, CTR) {
    try {

        RawPacketDataBuffer dataBuffer(MAX_PACKET_SIZE-1, MAX_PACKET_SIZE, MAX_PACKET_SIZE);

        ASSERT_TRUE(false);
    } catch (RawPacketDataParamOutOfRange& e) {
        ASSERT_TRUE(true);
    }
}

TEST_F(PacketDataBuffer, BufferOverflow) {

    try {
        char rawData[MAX_PACKET_SIZE];
        RawPacketDataBuffer dataBuffer(MAX_PACKET_SIZE*2, MAX_PACKET_SIZE, MAX_PACKET_SIZE);
        dataBuffer.writeRawData(rawData, MAX_PACKET_SIZE);
        dataBuffer.writeRawData(rawData, MAX_PACKET_SIZE);
        dataBuffer.writeRawData(rawData, MAX_PACKET_SIZE);
        ASSERT_TRUE(false);
    } catch (RawPacketDataBufferOverflow& e) {
        ASSERT_TRUE(true);
    } catch (exception& e) {
        ASSERT_TRUE(false);
    }
}

TEST_F(PacketDataBuffer, Simple) {
    char rawData[] = {          0xA3, 0x9D, 0x7A, // SYNC
                                0x01, // Message Type
                                0x00, 0x19,  // Message Size
                                0x00, 0x5D,  // Checksum
                                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // Time Stamp
                                0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09   // Payload
                              };
    stringstream out;

    printRawBytes(out, rawData, 0x19);
    LOG(DEBUG) << out.str();
    out.str("");

    RawPacket* rawPacket = reinterpret_cast<RawPacket*>(rawData);

    RawPacketDataBuffer dataBuffer(65536, MAX_PACKET_SIZE, MAX_PACKET_SIZE);

    dataBuffer.writeRawData(rawData, 0x19);

    Packet *packet = dataBuffer.getNextPacket();

    uint16_t rawDataSize = *((uint16_t*)(rawData+4));
    rawDataSize = ntohs(rawDataSize);
    ASSERT_EQ(rawDataSize, packet->packetSize());
    ASSERT_FALSE(memcmp(rawData, packet->packet(), packet->packetSize()));

    LOG(DEBUG) << out.str();
}

TEST_F(PacketDataBuffer, InvalidSync) {
    char rawData[] = {          0x7A, 0x9D, 0xA3, // Invalid SYNC
                                0x01, // Message Type
                                0x00, 0x19,  // Message Size
                                0x00, 0x5D,  // Checksum
                                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // Time Stamp
                                0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09   // Payload
                              };
    stringstream out;

    RawPacket* rawPacket = reinterpret_cast<RawPacket*>(rawData);

    LOG(DEBUG) << "checksum = " << rawPacket->calculateChecksum(rawPacket);

    RawPacketDataBuffer dataBuffer(65536, MAX_PACKET_SIZE, MAX_PACKET_SIZE);

    dataBuffer.writeRawData(rawData, 0x19);

    Packet *packet = dataBuffer.getNextPacket();
    ASSERT_NE(packet, (void*)NULL);

    printRawBytes(out, rawData, 0x19);
    LOG(DEBUG) << out.str();
    out.str("");
    printRawBytes(out, packet->payload(), packet->packetSize() - HEADER_SIZE);
    LOG(DEBUG) << out.str();

    uint16_t rawDataSize = *((uint16_t*)(rawData+4));
    rawDataSize = ntohs(rawDataSize);
    ASSERT_EQ(packet->packetType(), PORT_AGENT_FAULT);
    ASSERT_EQ(rawDataSize + HEADER_SIZE, packet->packetSize());
    ASSERT_FALSE(memcmp(rawData, packet->payload(), packet->packetSize() - HEADER_SIZE));
    delete packet;
    packet = NULL;

    packet = dataBuffer.getNextPacket();
    ASSERT_EQ(packet, (void*)NULL);  // No more packets

    out.str("");
    LOG(DEBUG) << out.str();
}


TEST_F(PacketDataBuffer, InvalidChecksum) {
    char rawData[] = {          0xA3, 0x9D, 0x7A,
                                0x01, // Message Type
                                0x00, 0x19,  // Message Size
                                0x00, 0x50,  // Checksum
                                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // Time Stamp
                                0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09   // Payload
                              };
    stringstream out;

    RawPacket* rawPacket = reinterpret_cast<RawPacket*>(rawData);

    LOG(DEBUG) << "InvalidChecksum: checksum = " << rawPacket->calculateChecksum(rawPacket);

    RawPacketDataBuffer dataBuffer(65536, MAX_PACKET_SIZE, MAX_PACKET_SIZE);

    dataBuffer.writeRawData(rawData, 0x19);

    Packet *packet = dataBuffer.getNextPacket();
    ASSERT_NE(packet, (void*)NULL);

    printRawBytes(out, rawData, 0x19);
    LOG(DEBUG) << out.str();
    out.str("");
    printRawBytes(out, packet->payload(), packet->packetSize() - HEADER_SIZE);
    LOG(DEBUG) << out.str();

    uint16_t rawDataSize = *((uint16_t*)(rawData+4));
    rawDataSize = ntohs(rawDataSize);
    ASSERT_EQ(packet->packetType(), PORT_AGENT_FAULT);
    ASSERT_EQ(rawDataSize + HEADER_SIZE, packet->packetSize());
    ASSERT_FALSE(memcmp(rawData, packet->payload(), packet->packetSize() - HEADER_SIZE));
    delete packet;
    packet = NULL;

    packet = dataBuffer.getNextPacket();
    ASSERT_EQ(packet, (void*)NULL);  // No more packets

    out.str("");
    LOG(DEBUG) << out.str();
}

TEST_F(PacketDataBuffer, InvalidHeader) {
    char rawData[] = {          0xA3, 0x9D, 0x7A,
                                0x00, // Message Type
                                0x00, 0x19,  // Message Size
                                0x00, 0x5D,  // Checksum
                                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // Time Stamp
                                0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09   // Payload
                              };
    stringstream out;

    RawPacket* rawPacket = reinterpret_cast<RawPacket*>(rawData);

    LOG(DEBUG) << "InvalidHeader: checksum = " << rawPacket->calculateChecksum(rawPacket);

    RawPacketDataBuffer dataBuffer(65536, MAX_PACKET_SIZE, MAX_PACKET_SIZE);

    dataBuffer.writeRawData(rawData, 0x19);

    Packet *packet = dataBuffer.getNextPacket();
    ASSERT_NE(packet, (void*)NULL);

    printRawBytes(out, rawData, 0x19);
    LOG(DEBUG) << out.str();
    out.str("");
    printRawBytes(out, packet->payload(), packet->packetSize() - HEADER_SIZE);
    LOG(DEBUG) << out.str();

    uint16_t rawDataSize = *((uint16_t*)(rawData+4));
    rawDataSize = ntohs(rawDataSize);
    ASSERT_EQ(packet->packetType(), PORT_AGENT_FAULT);
    ASSERT_EQ(rawDataSize + HEADER_SIZE, packet->packetSize());
    ASSERT_FALSE(memcmp(rawData, packet->payload(), packet->packetSize() - HEADER_SIZE));
    delete packet;
    packet = NULL;

    packet = dataBuffer.getNextPacket();
    ASSERT_EQ(packet, (void*)NULL);  // No more packets

    out.str("");
    LOG(DEBUG) << out.str();
}

TEST_F(PacketDataBuffer, LeadingInvalidData) {
    char rawData[] = {          0xA3, 0x12, 0x45, 0xA3, 0x00, 0xDF, 0x00,
                                0xA3, 0x9D, 0x7A, // SYNC
                                0x01, // Message Type
                                0x00, 0x19,  // Message Size
                                0x00, 0x5D,  // Checksum
                                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // Time Stamp
                                0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09  // Payload
                              };

    RawPacket* rawPacket = reinterpret_cast<RawPacket*>(rawData);

    RawPacketDataBuffer dataBuffer(65536, MAX_PACKET_SIZE, MAX_PACKET_SIZE);

    dataBuffer.writeRawData(rawData, 0x20);

    Packet *packet = dataBuffer.getNextPacket();
    // Should be an invalid packet
    ASSERT_NE(packet, (void*)NULL);
    ASSERT_EQ(packet->packetType(), PORT_AGENT_FAULT);
    uint16_t rawDataSize = 7;
    ASSERT_EQ(rawDataSize + HEADER_SIZE, packet->packetSize());
    ASSERT_FALSE(memcmp(rawData, packet->payload(), packet->packetSize() - HEADER_SIZE));
    delete packet;
    packet = NULL;

    packet = dataBuffer.getNextPacket();
    // Should be an valid packet
    ASSERT_NE(packet, (void*)NULL);
    ASSERT_EQ(packet->packetType(), DATA_FROM_INSTRUMENT);
    rawDataSize = 0x19;
    ASSERT_EQ(rawDataSize, packet->packetSize());
    ASSERT_FALSE(memcmp(rawData+7, packet->packet(), packet->packetSize()));
    delete packet;
    packet = NULL;

    packet = dataBuffer.getNextPacket();
    // Should be no more packets
    ASSERT_EQ(packet, (void*)NULL);  // No more packets
}

TEST_F(PacketDataBuffer, TrailingInvalidData) {
    char rawData[] = {          0xA3, 0x9D, 0x7A, // SYNC
                                0x01, // Message Type
                                0x00, 0x19,  // Message Size
                                0x00, 0x5D,  // Checksum
                                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // Time Stamp
                                0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09,  // Payload
                                0xA3, 0x12, 0x45, 0xA3, 0x00, 0xDF, 0x00
                              };

    RawPacket* rawPacket = reinterpret_cast<RawPacket*>(rawData);

    RawPacketDataBuffer dataBuffer(65536, MAX_PACKET_SIZE, MAX_PACKET_SIZE);

    dataBuffer.writeRawData(rawData, 0x20);

    Packet* packet = dataBuffer.getNextPacket();
    // Should be an valid packet
    ASSERT_NE(packet, (void*)NULL);
    ASSERT_EQ(packet->packetType(), DATA_FROM_INSTRUMENT);
    uint16_t rawDataSize = 0x19;
    ASSERT_EQ(rawDataSize, packet->packetSize());
    ASSERT_FALSE(memcmp(rawData, packet->packet(), packet->packetSize()));
    delete packet;
    packet = NULL;

    packet = dataBuffer.getNextPacket();
    // Should be an invalid packet
    ASSERT_NE(packet, (void*)NULL);
    ASSERT_EQ(packet->packetType(), PORT_AGENT_FAULT);
    rawDataSize = 0x07;
    ASSERT_EQ(rawDataSize + HEADER_SIZE, packet->packetSize());
    ASSERT_FALSE(memcmp(rawData+0x19, packet->payload(), packet->packetSize() - HEADER_SIZE));
    delete packet;
    packet = NULL;

    packet = dataBuffer.getNextPacket();
    // Should be no more packets
    ASSERT_EQ(packet, (void*)NULL);  // No more packets
}

TEST_F(PacketDataBuffer, TruncatedPacket) {
    char rawData[] = {          0xA3, 0x9D, 0x7A, // SYNC
                                0x01, // Message Type
                                0x00, 0x19,  // Message Size
                                0x00, 0x5D,  // Checksum
                                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // Time Stamp
                                0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09   // Payload
                              };

    RawPacketDataBuffer dataBuffer(65536, MAX_PACKET_SIZE, MAX_PACKET_SIZE);
    Packet* packet = NULL;

    // Truncated sync
    dataBuffer.writeRawData(rawData, 0x02);
    packet = dataBuffer.getNextPacket();
    ASSERT_EQ(packet, (void*)NULL);
    dataBuffer.clear();

    // Truncated header
    dataBuffer.writeRawData(rawData, 0x07);
    packet = dataBuffer.getNextPacket();
    ASSERT_EQ(packet, (void*)NULL);
    dataBuffer.clear();

    // Truncated payload
    dataBuffer.writeRawData(rawData, 0x14);
    packet = dataBuffer.getNextPacket();
    ASSERT_EQ(packet, (void*)NULL);
    dataBuffer.clear();

    // Partial packets between writes
    dataBuffer.writeRawData(rawData, 0x07);
    packet = dataBuffer.getNextPacket();
    ASSERT_EQ(packet, (void*)NULL);
    dataBuffer.writeRawData(rawData+0x07, 0x19 - 0x07);
    packet = dataBuffer.getNextPacket();
    ASSERT_NE(packet, (void*)NULL);
    uint16_t rawDataSize = *((uint16_t*)(rawData+4));
    rawDataSize = ntohs(rawDataSize);
    ASSERT_EQ(rawDataSize, packet->packetSize());
    ASSERT_FALSE(memcmp(rawData, packet->packet(), packet->packetSize()));
}

TEST_F(PacketDataBuffer, WrapAround) {
    char rawData[MAX_PACKET_SIZE];
    size_t bufferSize = 65536;

    RawPacketDataBuffer dataBuffer(bufferSize, MAX_PACKET_SIZE, MAX_PACKET_SIZE);
    Packet* packet = NULL;
    size_t totalBytesWritten = 0;

    while (totalBytesWritten < (bufferSize*10)) {
        size_t rawSize = buildRawPacket(rawData);
        totalBytesWritten += rawSize;
        dataBuffer.writeRawData(rawData, rawSize);
        packet = dataBuffer.getNextPacket();
        ASSERT_NE(packet, (void*)NULL);
        delete packet;
        packet = NULL;
    }
}
