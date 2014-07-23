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

#ifndef __RAW_PACKET_
#define __RAW_PACKET_

#include "raw_header.h"

namespace packet {

class RawPacket: public RawHeader {
    /********************
     *      METHODS     *
     ********************/
public:
    ///////////////////////
    // Public Methods

    /* Accessors */
    char* getPayload();

    // Validate checksum
    bool validateChecksum();

    // Calculate checksum of current packet
    uint16_t calculateChecksum(RawPacket* rawPacket);

private:
    ///////////////////////
    // Private Methods

    // Private constructor to restrict use
    RawPacket();

    // Private constructor to restrict use
    RawPacket(const RawPacket &rhs);

    // Private assignment operator to restrict use
    RawPacket& operator=(const RawPacket &rhs);

    /********************
     *      MEMBERS     *
     ********************/

    char rawPayload[];

};
}

#endif // __RAW_PACKET_
