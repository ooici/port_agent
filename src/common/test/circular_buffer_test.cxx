#include "circular_buffer.h"
#include "logger.h"
#include "util.h"
#include "gtest/gtest.h"

#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <string>

using namespace logger;
using namespace std;

class CircularBufferTest : public testing::Test {

    protected:
        virtual void SetUp() {
            Logger::SetLogFile("/tmp/gtest.log");
            Logger::SetLogLevel("DEBUG");

            LOG(INFO) << "************************************************";
            LOG(INFO) << "            CircularBufferTest Start Up";
            LOG(INFO) << "************************************************";
        }

        virtual void TearDown() {
            LOG(INFO) << "CircularBufferTest TearDown";
        }

        void const printRawBytes(stringstream& out, const char* buffer, const size_t numBytes) {
            for (size_t ii = 0; ii < numBytes;ii++)
                out << setfill('0') << setw(2) << hex << byteToUnsignedInt(buffer[ii]);
        }
};

/* Test  CircularBuffer constructor */
TEST_F(CircularBufferTest, CTR) {
    CircularBuffer circularBuffer(10);
    EXPECT_EQ(circularBuffer.capacity(), 10);
    EXPECT_EQ(circularBuffer.available(), 10);
    EXPECT_EQ(circularBuffer.size(), 0);
}

TEST_F(CircularBufferTest, ReadWrite) {

    uint32_t capacity = 5000;
    CircularBuffer circularBuffer(capacity);

    uint32_t dataSize = 10000;
    char writeData[dataSize];

    // Populate data
    for (uint32_t ii = 0; ii < dataSize; ii++) {
        writeData[ii] = (rand() % 256);
    }

    char readData[dataSize];

    // Add data
    size_t bytes_written = circularBuffer.write(writeData, 3500);
    EXPECT_EQ(bytes_written, 3500);
    EXPECT_EQ(circularBuffer.available(), 1500);
    EXPECT_EQ(circularBuffer.size(), 3500);

    // Fill up buffer by attempting to overflow
    bytes_written = circularBuffer.write(writeData+3500, 5000);
    EXPECT_EQ(bytes_written, 1500);
    EXPECT_EQ(circularBuffer.available(), 0);
    EXPECT_EQ(circularBuffer.size(), capacity);

    // Attempt to write more data
    bytes_written = circularBuffer.write(writeData, 100);
    EXPECT_EQ(bytes_written, 0);
    EXPECT_EQ(circularBuffer.available(), 0);
    EXPECT_EQ(circularBuffer.size(), capacity);

    // Read data from buffer
    memset(readData, 0, dataSize);
    size_t bytes_read = circularBuffer.read(readData, 1500);
    EXPECT_EQ(bytes_read, 1500);
    EXPECT_EQ(circularBuffer.available(), 1500);
    EXPECT_EQ(circularBuffer.size(), 3500);
    EXPECT_TRUE(memcmp(writeData, readData, 10000));
    EXPECT_FALSE(memcmp(writeData, readData, 1500));

    // Write more data to buffer, should wrap around
    bytes_written = circularBuffer.write(writeData + capacity, 1000);
    EXPECT_EQ(bytes_written, 1000);
    EXPECT_EQ(circularBuffer.available(), 500);
    EXPECT_EQ(circularBuffer.size(), 4500);

    // Read data from buffer, should wrap around
    memset(readData, 0, dataSize);
    bytes_read = circularBuffer.read(readData, 2500);
    EXPECT_EQ(bytes_read, 2500);
    EXPECT_EQ(circularBuffer.available(), 3000);
    EXPECT_EQ(circularBuffer.size(), 2000);
    EXPECT_TRUE(memcmp(writeData, readData, 10000));
    EXPECT_FALSE(memcmp(writeData + 1500, readData, 2500));

    // Clear buffer and attempt to read data
    circularBuffer.clear();
    memset(readData, 0, dataSize);
    EXPECT_EQ(circularBuffer.capacity(), capacity);
    EXPECT_EQ(circularBuffer.available(), capacity);
    EXPECT_EQ(circularBuffer.size(), 0);
    bytes_read = circularBuffer.read(readData, 1000);
    EXPECT_EQ(bytes_read, 0);

    // Attempt to read more data than size of buffer
    bytes_written = circularBuffer.write(writeData, 2000);
    EXPECT_EQ(bytes_written, 2000);
    EXPECT_EQ(circularBuffer.available(), 3000);
    EXPECT_EQ(circularBuffer.size(), 2000);
    memset(readData, 0, dataSize);
    bytes_read = circularBuffer.read(readData, 5000);
    EXPECT_EQ(bytes_read, 2000);
    EXPECT_EQ(circularBuffer.available(), 5000);
    EXPECT_EQ(circularBuffer.size(), 0);
    EXPECT_TRUE(memcmp(writeData, readData, 10000));
    EXPECT_FALSE(memcmp(writeData, readData, 2000));
}

TEST_F(CircularBufferTest, Discard) {
    uint32_t capacity = 5000;
    CircularBuffer circularBuffer(capacity);
    uint32_t dataSize = 10000;
    char writeData[dataSize];
    char readData[dataSize];

    // Populate data
    for (uint32_t ii = 0; ii < dataSize; ii++) {
        writeData[ii] = (rand() % 256);
    }

    // Fill up buffer
    size_t bytes_written = circularBuffer.write(writeData, capacity);
    EXPECT_EQ(bytes_written, capacity);
    EXPECT_EQ(circularBuffer.available(), 0);
    EXPECT_EQ(circularBuffer.size(), capacity);

    // Discard data
    size_t bytes_discarded = circularBuffer.discard(1000);
    EXPECT_EQ(bytes_discarded, 1000);
    EXPECT_EQ(circularBuffer.available(), 1000);
    EXPECT_EQ(circularBuffer.size(), 4000);

    // Read data
    memset(readData, 0, dataSize);
    size_t bytes_read = circularBuffer.read(readData, 1000);
    EXPECT_EQ(bytes_read, 1000);
    EXPECT_FALSE(memcmp(writeData+1000, readData, 1000));

    // Write data to wrap around
    bytes_written = circularBuffer.write(writeData+capacity, 1000);
    EXPECT_EQ(bytes_written, 1000);
    EXPECT_EQ(circularBuffer.available(), 1000);
    EXPECT_EQ(circularBuffer.size(), 4000);

    // Discard data
    bytes_discarded = circularBuffer.discard(500);
    EXPECT_EQ(bytes_discarded, 500);
    EXPECT_EQ(circularBuffer.available(), 1500);
    EXPECT_EQ(circularBuffer.size(), 3500);

    // Read data
    memset(readData, 0, dataSize);
    bytes_read = circularBuffer.read(readData, 3000);
    EXPECT_EQ(bytes_read, 3000);
    EXPECT_FALSE(memcmp(writeData+2500, readData, 3000));

    // Discard more data than buffer size
    bytes_discarded = circularBuffer.discard(10000);
    EXPECT_EQ(bytes_discarded, 500);
    EXPECT_EQ(circularBuffer.available(), 5000);
    EXPECT_EQ(circularBuffer.size(), 0);

    // Attempt to discard empty buffer
    bytes_discarded = circularBuffer.discard(500);
    EXPECT_EQ(bytes_discarded, 0);
    EXPECT_EQ(circularBuffer.available(), 5000);
    EXPECT_EQ(circularBuffer.size(), 0);
}

TEST_F(CircularBufferTest, Peek) {
    uint32_t capacity = 5000;
    CircularBuffer circularBuffer(capacity);
    uint32_t dataSize = 10000;
    char writeData[dataSize];
    char readData[dataSize];

    // Populate data
    for (uint32_t ii = 0; ii < dataSize; ii++) {
        writeData[ii] = (rand() % 256);
    }

    // Fill up buffer
    size_t bytes_written = circularBuffer.write(writeData, capacity);
    EXPECT_EQ(bytes_written, capacity);
    EXPECT_EQ(circularBuffer.available(), 0);
    EXPECT_EQ(circularBuffer.size(), capacity);

    // Peek at data
    memset(readData, 0, dataSize);
    size_t peek_bytes = circularBuffer.peek(readData, 1000);
    EXPECT_EQ(peek_bytes, 1000);
    EXPECT_FALSE(memcmp(writeData, readData, 1000));

    // Peek again
    memset(readData, 0, dataSize);
    peek_bytes = circularBuffer.peek(readData, 500);
    EXPECT_EQ(peek_bytes, 500);
    EXPECT_FALSE(memcmp(writeData+1000, readData, 500));

    // Peek greater than size
    memset(readData, 0, dataSize);
    peek_bytes = circularBuffer.peek(readData, 5000);
    EXPECT_EQ(peek_bytes, 3500);
    EXPECT_FALSE(memcmp(writeData+1500, readData, 3500));

    // Read data, peek should reset
    memset(readData, 0, dataSize);
    circularBuffer.read(readData, 1000);
    EXPECT_FALSE(memcmp(writeData, readData, 1000));

    // Peek
    memset(readData, 0, dataSize);
    peek_bytes = circularBuffer.peek(readData, 500);
    EXPECT_EQ(peek_bytes, 500);
    EXPECT_FALSE(memcmp(writeData+1000, readData, 500));

    // Discard data, peek should reset
    circularBuffer.discard(1000);

    // Peek again
    memset(readData, 0, dataSize);
    peek_bytes = circularBuffer.peek(readData, 1000);
    EXPECT_EQ(peek_bytes, 1000);
    EXPECT_FALSE(memcmp(writeData+2000, readData, 1000));

    // Write then peek
    bytes_written = circularBuffer.write(writeData+capacity, 1000);
    EXPECT_EQ(bytes_written, 1000);
    EXPECT_EQ(circularBuffer.available(), 1000);
    EXPECT_EQ(circularBuffer.size(), 4000);
    memset(readData, 0, dataSize);
    peek_bytes = circularBuffer.peek(readData, 1000);
    EXPECT_EQ(peek_bytes, 1000);
    EXPECT_FALSE(memcmp(writeData+3000, readData, 1000));

    // Clear buffer and attempt to peek
    circularBuffer.clear();
    peek_bytes = circularBuffer.peek(readData, 1000);
    EXPECT_EQ(peek_bytes, 0);
}

TEST_F(CircularBufferTest, BytePeek) {
    uint32_t capacity = 5000;
    CircularBuffer circularBuffer(capacity);
    uint32_t dataSize = 10000;
    char writeData[dataSize];
    char readData[dataSize];
    char peek_byte = 0;

    // Populate data
    for (uint32_t ii = 0; ii < dataSize; ii++) {
        writeData[ii] = (rand() % 256);
    }

    // Fill up buffer
    size_t bytes_written = circularBuffer.write(writeData, capacity);
    EXPECT_EQ(bytes_written, capacity);
    EXPECT_EQ(circularBuffer.available(), 0);
    EXPECT_EQ(circularBuffer.size(), capacity);

    // Peek at data
    peek_byte = 0;
    size_t peek_bytes = circularBuffer.peek_next_byte(peek_byte);
    stringstream out;
    printRawBytes(out, &peek_byte, 1);
    out << ", ";
    printRawBytes(out, writeData, 1);
    LOG(INFO) << "peek byte, writeData[0] = " << out.str();
    EXPECT_EQ(peek_bytes, 1);
    EXPECT_FALSE(memcmp(&peek_byte, writeData + 0, 1));

    // Peek again
    peek_byte = 0;
    peek_bytes = circularBuffer.peek_next_byte(peek_byte);
    EXPECT_EQ(peek_bytes, 1);
    EXPECT_FALSE(memcmp(&peek_byte, writeData + 1, 1));

    // Read data
    memset(readData, 0, dataSize);
    circularBuffer.read(readData, 1000);
    EXPECT_FALSE(memcmp(writeData, readData, 1000));

    // Peek
    memset(readData, 0, dataSize);
    peek_bytes = circularBuffer.peek_next_byte(peek_byte);
    EXPECT_EQ(peek_bytes, 1);
    EXPECT_EQ(peek_byte, writeData[1000]);

    // Discard data
    circularBuffer.discard(1000);

    // Peek again
    memset(readData, 0, dataSize);
    peek_bytes = circularBuffer.peek_next_byte(peek_byte);
    EXPECT_EQ(peek_bytes, 1);
    EXPECT_EQ(peek_byte, writeData[2000]);

    // Clear buffer and attempt to peek
    circularBuffer.clear();
    peek_byte = 0;
    peek_bytes = circularBuffer.peek_next_byte(peek_byte);
    EXPECT_EQ(peek_bytes, 0);
    EXPECT_EQ(peek_byte, 0);
}

