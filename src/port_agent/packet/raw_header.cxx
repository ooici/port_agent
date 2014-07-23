/*******************************************************************************
 * Class: RawHeader
 * Filename: raw_header.h
 * Author: Kevin Stiemke (stiemke27@gmail.com)
 * License: Apache 2.0
 *
 * A port agent packet wrapper for use with raw byte data.
 *
 * Note: It is assumed the raw byte data has big endian byte ordering.
 *
 * A packet contains:
 *
 * sync series      24 bits
 * message type     8 bits
 * packet size      16 bits (including the header)
 * checksum         16 bits
 * timestamp        64 bits
 * payload          variable size
 *
 * Usage:
 *
 * char* rawData;
 * read(rawData);
 * RawHeader* rawHeader = reinterpret_cast<RawHeader*>(rawData);
 *
 ******************************************************************************/

#include "raw_header.h"
#include "common/logger.h"
#include "common/util.h"

#include <sys/types.h>
#include <unistd.h>
#include <string>
#include <stdlib.h>

using namespace logger;
using namespace packet;
using namespace std;

/******************************************************************************
 * Method: getSync
 * Description: return sync
 *
 * Return:
 *   sync converted to little endian
 ******************************************************************************/
const uint32_t RawHeader::getSync() const {
    return (ntohl(sync_) >> 8);
}

/******************************************************************************
 * Method: getPacketType
 * Description: return packet type
 *
 * Return:
 *   packet type enumeration
 ******************************************************************************/
const PacketType RawHeader::getPacketType() const {
    const PacketType packetType =
            static_cast<const PacketType>(packetType_);
    return packetType;
}

/******************************************************************************
 * Method: getPacketSize
 * Description: return packet size
 *
 * Return:
 *   packet size converted to little endian
 ******************************************************************************/
const uint16_t RawHeader::getPacketSize() const {
    return ntohs(packetSize_);
}

/******************************************************************************
 * Method: getChecksum
 * Description: return checksum
 *
 * Return:
 *   checksum converted to little endian
 ******************************************************************************/
const uint16_t RawHeader::getChecksum() const {
    return ntohs(checksum_);
}

/******************************************************************************
 * Method: getTimestamp
 * Description: Convert raw timestamp to Timestamp
 *
 * Return:
 *   Timestamp converted from raw data
 ******************************************************************************/
const Timestamp RawHeader::getTimestamp() const {
    uint32_t seconds = ntohl(timestamp_ & 0x00000000FFFFFFFF);
    uint32_t fraction = ntohl((timestamp_ & 0xFFFFFFFF00000000) >> 32);
    const Timestamp timestamp(seconds, fraction);
    return timestamp;
}

/******************************************************************************
 * Method: getPayloadSize
 * Description: return raw payload size
 *
 * Return:
 *   size of payload
 ******************************************************************************/
const uint16_t RawHeader::getPayloadSize() const {
    return (getPacketSize() - HEADER_SIZE);
}

/******************************************************************************
 * Method: validateHeader
 * Description: validate raw header
 * Parameters:
 *   maxPacketSize - maximum packet size
 *   packetType - expected packet type
 * Return:
 *   true if header is valid
 ******************************************************************************/
const bool RawHeader::validateHeader(const size_t maxPacketSize) const {

    bool valid = true;

    if (getSync() != SYNC) {
        LOG(DEBUG) << "Invalid SYNC = " << hex << uppercase << getSync();
        valid = false;
    }

    /* Known packet types from packet.h
    enum PacketType {
        UNKNOWN,
        DATA_FROM_INSTRUMENT,
        DATA_FROM_DRIVER,
        PORT_AGENT_COMMAND,
        PORT_AGENT_STATUS,
        PORT_AGENT_FAULT,
        INSTRUMENT_COMMAND,
        PORT_AGENT_HEARTBEAT
    };  */
    PacketType packetType = getPacketType();
    if ((packetType < DATA_FROM_INSTRUMENT) || (packetType > PORT_AGENT_HEARTBEAT))
    {
        LOG(DEBUG) << "Invalid Packet Type = " << packetType;
        valid = false;
    }

    if ((getPacketSize() < HEADER_SIZE)
            || (getPacketSize() > maxPacketSize)) {
        LOG(DEBUG) << "Invalid Packet Size = " << hex << uppercase << getPacketSize();
        valid = false;
    }

    return valid;
}
