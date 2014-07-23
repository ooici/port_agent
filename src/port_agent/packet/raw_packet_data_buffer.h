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

#ifndef __RAW_PACKET_DATA_BUFFER_H_
#define __RAW_PACKET_DATA_BUFFER_H_

#define SYNC_SIZE 3
#define SYNC_MAX_INDEX 4
#define SYNC_MIN_INDEX 1

#include "common/circular_buffer.h"
#include "packet.h"
#include "raw_packet.h"

#include <stddef.h>
#include <sys/types.h>

using namespace std;

namespace packet {

class RawPacketDataBuffer : public CircularBuffer {
        /********************
         *      METHODS     *
         ********************/

    public:
        ///////////////////////
        // Public Methods

        // Only constructor
        RawPacketDataBuffer(size_t bufferCapacity, size_t maxPacketSize, size_t maxInvalidDataSize);

        // Destructor
        ~RawPacketDataBuffer();

        // Get next packet in buffer
        Packet* getNextPacket();

        // Write data to buffer
        void writeRawData(const char *data, size_t bytes);

    private:

        // Private to prevent usage
        RawPacketDataBuffer();

        // Get any leading invalid data in the buffer
        size_t getAnyLeadingInvalidData(char* data, bool invalidSync);

        // Read port agent packet header from buffer without removing
        const size_t peekHeader(char* data);

        // Read port agent packet from buffer without removing
        const size_t peekPacket(char* data, size_t packetSize);

        // Check for leading invalid data and create packet
        Packet* checkForInvalidPacket(bool invalidSync = false);

        // Check for leading valid data and create packet
        Packet* checkForPacket();

        /********************
         *      MEMBERS     *
         ********************/

        // Maximum port agent packet size
        size_t maxPacketSize_;

        // Maximum invalid port agent packet size
        size_t maxInvalidDataSize_;

        // Big endian sync
        const uint32_t nlSync;

        // Big endian sync bytes
        const char* syncChar;

    };
}

#endif // __RAW_PACKET_DATA_BUFFER_H_
