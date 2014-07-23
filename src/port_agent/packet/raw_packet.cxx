/*******************************************************************************
 * Class: RawPacket
 * Filename: raw_packet.h
 * Author: Kevin Stiemke (stiemke27@gmail.com)
 * License: Apache 2.0
 *
 * A port agent packet wrapper for use with raw byte data.
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
 * RawPacket* rawPacket = reinterpret_cast<RawPacket*>(rawData);
 *
 * Packet packet(rawPacket.getPacketType(),
 *              rawPacket.getTimestamp(),
 *              rawPacket.getPayload(),
 *              rawPacket.getPacketSize() - HEADER_SIZE);
 *
 * if(packet.readyToSend())
 *    write(packet.packet(), packet().packetSize());
 *
 ******************************************************************************/

#include "raw_packet.h"
#include "common/util.h"
#include "common/logger.h"

using namespace logger;
using namespace packet;

/******************************************************************************
 * Method: getPayload
 * Description: return a pointer to the raw packet payload
 *
 * Return:
 *   a char* to the raw packet payload
 ******************************************************************************/
char* RawPacket::getPayload() {
    if (getPacketSize() >= HEADER_SIZE)
        return rawPayload;
    else
        return NULL;
}

/******************************************************************************
 * Method: validateChecksum
 * Description: Calculate checksum and compare to current value
 *
 * Return:
 *   true if checksum is correct
 ******************************************************************************/
bool RawPacket::validateChecksum() {

    uint16_t checksum = getChecksum();
    uint16_t calculatedChecksum = calculateChecksum(this);

    LOG(DEBUG) << "checksum, calculated checksum = " << checksum << ", " << calculatedChecksum;

    return (checksum == calculatedChecksum);
}

/******************************************************************************
 * Method: calculateChecksum
 * Description: Calculate checksum
 * Parameters:
 *   rawPacket - raw packet to calculate it's checksum
 * Return:
 *   checksum
 ******************************************************************************/
uint16_t RawPacket::calculateChecksum(RawPacket* rawPacket) {
    uint16_t checksum = 0;

    char* m_pPacket = reinterpret_cast<char*>(this);

    for(int i = 0; i < getPacketSize(); i++) {
        // Make sure we ignore the part of the buffer where we store the
        // checksum value.
        if(i < 6 || i > 7) {
            checksum = checksum ^ byteToUnsignedInt(m_pPacket[i]);
        }
    }

    return checksum;
}
