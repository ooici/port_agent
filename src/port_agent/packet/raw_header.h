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

#ifndef __RAW_HEADER_H_
#define __RAW_HEADER_H_

#include "packet.h"

#include <stdint.h>

namespace packet {

class RawHeader {
    /********************
     *      METHODS     *
     ********************/

public:
    ///////////////////////
    // Public Methods

    /* Accessors */
    const uint32_t getSync() const;
    const PacketType getPacketType() const;
    const uint16_t getPacketSize() const;
    const uint16_t getChecksum() const;
    const Timestamp getTimestamp() const;
    const uint16_t getPayloadSize() const;


    const bool validateHeader(const size_t maxPacketSize) const;

private:
    ///////////////////////
    // Private Methods

    // Private constructor to restrict use
    RawHeader();

    // Private constructor to restrict use
    RawHeader(const RawHeader &rhs);

    // Private assignment operator to restrict use
    RawHeader& operator=(const RawHeader &rhs);

    /********************
     *      MEMBERS     *
     ********************/

    uint32_t sync_ :24;
    uint8_t packetType_ :8;
    uint16_t packetSize_ :16;
    uint16_t checksum_ :16;
    uint64_t timestamp_ :64;
};
}

#endif // __RAW_HEADER_H_
