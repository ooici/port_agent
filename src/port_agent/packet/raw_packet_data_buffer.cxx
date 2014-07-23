/*******************************************************************************
 * Class: RawPacketDataBuffer
 * Filename: raw_packet_data_buffer.h
 * Author: Kevin Stiemke (stiemke27@gmail.com)
 * License: Apache 2.0
 *
 * A buffer for raw port agent data packets.  The buffer takes arbitrary length
 * data and composes port agent packets from the data.
 *
 * Usage:
 *
 * RawPacketDataBuffer rawPacketDataBuffer(capacity, maxPacketSize, maxInvalidDataSize);
 *
 * char* rawData;
 * rawDataSize = read(rawData);
 *
 * rawPacketDataBuffer.writeRawData(rawData, rawDataSize);
 *
 * Packet* packet = rawPacketDataBuffer.getNextPacket()
 *
 * delete packet;
 *
 ******************************************************************************/

#include "raw_packet_data_buffer.h"
#include "common/logger.h"
#include "common/util.h"
#include "common/exception.h"

#include <string>
#include <sys/types.h>
#include <unistd.h>

using namespace logger;
using namespace packet;
using namespace std;

/******************************************************************************
 * Method: Constructor
 * Description: Only constructor for this class
 * Parameters:
 *   bufferCapacity - Maximum size of the buffer
 *   maxPacketSize - Maximum port agent packet size
 *   maxInvalidDataSize - Maximum invalid port agent packet size
 * Throws:
 *   RawPacketDataParamOutOfRange - maxPacketSize is greater than
 *     bufferCapacity
 *
 ******************************************************************************/
RawPacketDataBuffer::RawPacketDataBuffer(size_t bufferCapacity, size_t maxPacketSize, size_t maxInvalidDataSize):
    CircularBuffer(bufferCapacity),
    maxPacketSize_(maxPacketSize),
    maxInvalidDataSize_(maxInvalidDataSize),
    nlSync(htonl(SYNC)),
    syncChar(reinterpret_cast<const char*>(&nlSync)) {

    if (maxPacketSize_ > bufferCapacity) {
        throw RawPacketDataParamOutOfRange("Packet size greater than capacity");
    }

    if (maxInvalidDataSize_ > maxPacketSize_) {
        maxInvalidDataSize_ = maxPacketSize_;
    }
}

/******************************************************************************
 * Method: Destructor
 * Description: Destructor
 *
 ******************************************************************************/
RawPacketDataBuffer::~RawPacketDataBuffer() {

}

/******************************************************************************
 * Method: writeRawData
 * Description: Write data to the buffer.  Differentiated between the write in
 *   the base class by throwing an exception if buffer is full or a write
 *   error occurs.
 * Parameters:
 *   data - Raw data to buffer
 *   bytes - Size of raw data
 * Throws:
 *   RawPacketDataBufferOverflow - buffer is full
 *   RawPacketDataWriteError - unexpected error occurred writing to buffer
 *
 ******************************************************************************/
void RawPacketDataBuffer::writeRawData(const char *data, size_t bytes) {
    size_t bytesWritten = write(data, bytes);

    if (bytesWritten < bytes) {
        throw RawPacketDataBufferOverflow();
    }

    if (bytesWritten > bytes) {
        throw RawPacketDataWriteError();
    }
}

/******************************************************************************
 * Method: checkForInvalidPacket
 * Description: Check buffer for leading invalid data.  If leading invalid data
 *   is found a invalid data port agent packet is created.
 * Parameters:
 *   invalidSync - indicates the leading sync is invalid
 * Return:
 *   NULL pointer if no invalid data or pointer to dynamically allocated
 *     invalid data port agent
 *     packet
 * Throws:
 *   RawPacketDataReadError - unexpected error occurred reading from buffer
 *
 ******************************************************************************/
Packet* RawPacketDataBuffer::checkForInvalidPacket(bool invalidSync) {
    Packet* packet = NULL;
    char data[maxPacketSize_];

    size_t numberInvalidBytes = getAnyLeadingInvalidData(data, invalidSync);  // Get invalid data until a sync appears

    LOG(DEBUG) << "Number invalid bytes = " << numberInvalidBytes;

    if (numberInvalidBytes > 0)
    {
        RawPacket* rawPacket = reinterpret_cast<RawPacket*>(data);
        packet = new Packet(PORT_AGENT_FAULT, Timestamp(), data, numberInvalidBytes);  // TODO: new packet type?
    }

    return packet;
}

/******************************************************************************
 * Method: checkForPacket
 * Description: Check buffer for port agent packet and validate.  If
 *   validation fails, the buffer is checked for invalid data.
 * Return: NULL pointer if no packet or pointer to dynamically allocated port
 *   agent packet.
 * Throws:
 *   RawPacketDataReadError - unexpected error occurred reading from buffer
 *
 ******************************************************************************/
Packet* RawPacketDataBuffer::checkForPacket() {
    Packet* packet = NULL;
    char data[maxPacketSize_];

    if (size() < HEADER_SIZE) {
        LOG(DEBUG) << "Header possibly truncated";
        return packet;
    }

    peekHeader(data);

    RawHeader* header = reinterpret_cast<RawHeader*>(data);

    if (header->validateHeader(maxPacketSize_)) {
        if (header->getPacketSize() <= size()) {
            peekPacket(data, header->getPacketSize());
            RawPacket* rawPacket = reinterpret_cast<RawPacket*>(data);
            if (rawPacket->validateChecksum()) {
                packet = new Packet(header->getPacketType(), rawPacket->getTimestamp(), rawPacket->getPayload(), rawPacket->getPayloadSize());
                size_t bytesDiscarded = discard(rawPacket->getPacketSize());
                if (bytesDiscarded != rawPacket->getPacketSize()) {
                    throw RawPacketDataReadError();
                }
            } else {
                // TODO: Throw packet away unless it contains a sync?
                LOG(DEBUG) << "Invalid checksum, throw whole packet away";
                packet = checkForInvalidPacket(true);
            }
        } else {
            LOG(DEBUG) << "Packet possibly truncated";
            packet = NULL;
        }
    } else {
        LOG(DEBUG) << "Invalid header";
        // TODO: Throw header away unless it contains a sync?
        packet = checkForInvalidPacket(true);
    }

    return packet;
}

/******************************************************************************
 * Method: getNextPacket
 * Description: Get next port agent packet from buffer.  Port agent packets
 *   are dynamically allocated and the caller is responsible for deleting
 *   the memory.
 * Return: NULL pointer if no packet or pointer to dynamically allocated port
 *   agent packet.
 * Throws:
 *   RawPacketDataReadError - unexpected error occurred reading from buffer
 *
 ******************************************************************************/
Packet* RawPacketDataBuffer::getNextPacket() {

    Packet* packet = NULL;

    LOG(DEBUG) << "getNextPacket(): buffer size = " << size();

    // Buffer is empty
    if (size() == 0) {
        LOG(DEBUG) << "No packets, buffer size = 0";
        return packet;
    }

    //  First check for leading invalid data
    packet = checkForInvalidPacket();

    if (packet == NULL) {
        // No leading invalid data, buffer empty or at sync word
        packet = checkForPacket();
    } else {
        LOG(DEBUG) << "Invalid packet";
    }

    if (packet != NULL) {
        LOG(MESG) << endl << "Begin Pretty Print Packet" << packet->pretty() << endl << "End Pretty Print Packet";
        LOG(DEBUG) << "Packet created, buffer size = " << size();
    } else if (packet == NULL) {
        LOG(DEBUG) << endl << "No packets, buffer size = " << size();
    }

    return packet;
}

/******************************************************************************
 * Method: getAnyLeadingInvalidData
 * Description: Get any leading invalid data from buffer.  Leading invalid data
 *   is any data before a valid sync.
 * Parameters:
 *   data - Buffer to write invalid data into.
 *   invalidSync - indicates the buffer starts with an invalid sync
 * Return: size of invalid data read.
 * Throws:
 *   RawPacketDataReadError - unexpected error occurred reading from buffer
 *
 ******************************************************************************/
size_t RawPacketDataBuffer::getAnyLeadingInvalidData(char* data, bool invalidSync) {

    size_t numberInvalidBytes = 0;
    char next_byte;
    size_t sync_index = SYNC_MIN_INDEX;

    if (invalidSync && (size() >= SYNC_SIZE)) {
        // TODO: Will peek throw things off? shouldn't
        numberInvalidBytes = peek(data, SYNC_SIZE);
        if (numberInvalidBytes != SYNC_SIZE) {
            throw RawPacketDataReadError();
        }
    }

    while(peek_next_byte(next_byte))
    {
        if (next_byte == syncChar[sync_index])
        {
            LOG(DEBUG) << "Found sync character = " << byteToUnsignedInt(syncChar[sync_index]);
            sync_index++;
            if (sync_index == SYNC_MAX_INDEX) {
                LOG(DEBUG) << "Found full sync";
                break;
            }
        }
        else
        {
            numberInvalidBytes++;
            numberInvalidBytes += sync_index - SYNC_MIN_INDEX;
            sync_index = SYNC_MIN_INDEX;
            if (numberInvalidBytes > maxInvalidDataSize_) {
                LOG(DEBUG) << "Reached maximum invalid data size";
                break;
            }
        }
    }

    if ((sync_index > SYNC_MIN_INDEX) && (sync_index < SYNC_MAX_INDEX)) {
        LOG(DEBUG) << "Sync possibly truncated.";
    }

    reset_peek();

    if (numberInvalidBytes > 0) {
        size_t bytesRead = read(data, numberInvalidBytes);
        if (bytesRead != numberInvalidBytes) {
            throw RawPacketDataReadError();
        }
    }

    return numberInvalidBytes;
}

/******************************************************************************
 * Method: peekHeader
 * Description: Read port agent header data from buffer without removing
 * Parameters:
 *   data - Buffer to write header data into.
 * Return: size of port agent header
 * Throws:
 *   RawPacketDataReadError - unexpected error occurred reading from buffer
 *
 ******************************************************************************/
const size_t RawPacketDataBuffer::peekHeader(char* data) {
    size_t bytesRead = peek(data, HEADER_SIZE);
    reset_peek();
    if (bytesRead != HEADER_SIZE) {
        throw RawPacketDataReadError();
    }
    return bytesRead;
}

/******************************************************************************
 * Method: peekPacket
 * Description: Read port agent packet data from buffer without removing
 * Parameters:
 *   data - Buffer to write packet data into.
 * Return: size of port agent packet
 * Throws:
 *   RawPacketDataReadError - unexpected error occurred reading from buffer
 *
 ******************************************************************************/
const size_t RawPacketDataBuffer::peekPacket(char* data, size_t packetSize) {
    size_t bytesRead = peek(data, packetSize);
    reset_peek();
    if (bytesRead != packetSize) {
        throw RawPacketDataReadError();
    }
    return bytesRead;
}
