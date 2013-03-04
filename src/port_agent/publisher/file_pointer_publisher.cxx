/*******************************************************************************
 * Class: FilePointerPublisher
 * Filename: file_pointer_publisher.cxx
 * Author: Bill French (wfrench@ucsd.edu)
 * License: Apache 2.0
 *
 * This is a base packet for publishing packets to a c-style FILE*.  This
 * should work for tcp, udp, and serial connections.
 *
 * We will default to all handlers writting all packet data to the file pointer.
 * The specialized classes can disable handlers that they don't want.
 *
 ******************************************************************************/

#include "file_pointer_publisher.h"
#include "common/logger.h"
#include "common/exception.h"
#include "port_agent/packet/packet.h"

#include <sstream>
#include <string>
#include <stdio.h>
#include <string.h>
#include <errno.h>

using namespace std;
using namespace packet;
using namespace logger;
using namespace publisher;
    
/******************************************************************************
 *   PUBLIC METHODS
 ******************************************************************************/

/******************************************************************************
 * Method: Constructor
 * Description: Set the comm object
 * Parameter:
 *    CommBase* pointer to a comm object
 ******************************************************************************/
FilePointerPublisher::FilePointerPublisher() : Publisher() {
    m_pFilePointer = NULL;
    m_pCommSocket = NULL;
}

/******************************************************************************
 * Method: Copy Constructor
 * Description: Deep copy
 ******************************************************************************/
FilePointerPublisher::FilePointerPublisher(const FilePointerPublisher &rhs) : Publisher(rhs) {
	LOG(DEBUG) << "FilePointerPublisher copy ctor";
	
    m_pFilePointer = rhs.m_pFilePointer;
    m_pCommSocket = rhs.m_pCommSocket;
}

/******************************************************************************
 * Method: Constructor
 * Description: Set the comm object
 * Parameter:
 *    CommBase* pointer to a comm object
 ******************************************************************************/
FilePointerPublisher::FilePointerPublisher(CommBase* comm) {
    m_pFilePointer = NULL;
    setCommObject(comm);
}

/******************************************************************************
 * Method: Copy Constructor
 * Description: Copy constructor ensuring we do a deep copy 
 *
 * Parameters:
 *   copy - rhs object to copy
 *
 ******************************************************************************/
FilePointerPublisher & FilePointerPublisher::operator=(const FilePointerPublisher &rhs) {
    LOG(DEBUG2) << "FilePointerPublisher assignment operator";
	m_pFilePointer = rhs.m_pFilePointer;
    setCommObject(rhs.m_pCommSocket);
	clearError();
	return *this;
}

/******************************************************************************
 * Method: equality operator
 * Description: Are we the same
 *
 * Parameters:
 *   copy - rhs object to copy
 *
 ******************************************************************************/
bool FilePointerPublisher::operator==(FilePointerPublisher &rhs) {
	return compare(&rhs);
}

/******************************************************************************
 * Method: Compare
 * Description: Are we the same
 *
 * Parameters:
 *   copy - rhs object to copy
 *
 ******************************************************************************/
bool FilePointerPublisher::compare(Publisher *rhs) {
	LOG(DEBUG) << "FilePointerPublisher equality test";
	FilePointerPublisher *target = (FilePointerPublisher *)rhs;
	
	if(this == target) return true;
	if(!target) return false;

    LOG(DEBUG) << "Verify types match";
	if(publisherType() != target->publisherType())
        return false;
	
    return compareCommSocket(target->m_pCommSocket);
}


/******************************************************************************
 * Method: compare comm socket
 * Description: see if two comm sockets look the same
 *
 * Parameters:
 *   rhs object to compare
 *
 ******************************************************************************/
bool FilePointerPublisher::compareCommSocket(CommBase *rhs) {
	LOG(DEBUG) << "compare comm socket";
	
	// Both null
	if(!m_pCommSocket && !rhs)
	    return true;
	
    // one null
    if(!m_pCommSocket || !rhs)
	    return false;
	
    LOG(DEBUG) << "LHS Type: " << m_pCommSocket->type();
    LOG(DEBUG) << "RHS Type: " << rhs->type();
	
    // neither null.  do compare
	if(m_pCommSocket->type() != rhs->type())
	    return false;
	
	return m_pCommSocket->compare(rhs);
}


/******************************************************************************
 * Method: Destructor
 * Description: Clear dynamic memory allocated
 ******************************************************************************/
FilePointerPublisher::~FilePointerPublisher() {
}

/******************************************************************************
 * Method: setFilePointer
 * Description: Set the private file pointer (deep copy)
 * Parameter:
 *    FILE* referencing a file descriptor.
 ******************************************************************************/
void FilePointerPublisher::setCommObject(CommBase* comm) {
    m_pCommSocket = comm;
}

/******************************************************************************
 * Method: setFilePointer
 * Description: Set the private file pointer
 * Parameter:
 *    FILE* referencing a file descriptor.
 ******************************************************************************/
void FilePointerPublisher::setFilePointer(FILE* file) {
    m_pFilePointer = file;
}

/******************************************************************************
 * Method: logPacket
 * Description: Write a packet of data to the internal file pointer.  Raise an
 * error if we don't have a file pointer.
 *
 * Parameter:
 *    Packet* - Pointer to a packet of data we need to write to the FILE*
 ******************************************************************************/
bool FilePointerPublisher::logPacket(Packet *packet) {
    string output;

	if(m_bAsciiOut) {
        output = packet->asAscii();
        return write(output.c_str(), output.length());
    }

	// Must be binary
	return write(packet->packet(), packet->packetSize());
}

/******************************************************************************
 * Method: write
 * Description: Write a buffer the the internal FILE*.  It attempts to write
 * the buffer three times.  Exceptions are thrown if the FILE* is not set or
 * we fail to write the entire packet.
 *
 * Parameter:
 *    char* - the buffer that we are writing.
 *    size - how many bytes?
 *
 * Exceptions:
 *    FileDescriptorNULL
 *    PacketPublishFailure
 ******************************************************************************/
bool FilePointerPublisher::write(const char *buffer, uint32_t size) {
	int count;
	int total = 0;

	if(size == 0) {
		LOG(INFO) << "Empty buffer for write, bailing";
	    return false;
	}
	
	LOG(DEBUG) << "Write data byte count: " << size;

	if(m_pFilePointer == NULL && m_pCommSocket == NULL)
		throw FileDescriptorNULL();
	
    if (m_pCommSocket && ! m_pCommSocket->connected()) {
		LOG(DEBUG) << "Not connected.";
	    m_pCommSocket->connectClient();
    }

	// Try to write data three times.  Throw an error if we fail.
	for( int i = 0; i < 3 && total < size; i++) {
		LOG(DEBUG2) << "Packet write attempt #" << i+1;
		
		if(m_pCommSocket) {
			LOG(DEBUG2) << "write with comm socket.";
		    total += m_pCommSocket->writeData(buffer + total, size - total);
		}
		else if(m_pFilePointer) {
			LOG(DEBUG2) << "write with file pointer";
		    total += fwrite(buffer + total, 1, size - total, m_pFilePointer);
		}
		else {
			LOG(ERROR) << "socket not intialized! Boom!";
			throw PacketPublishFailure("socket not initialized");
		}
		
		LOG(DEBUG2) << "write attempt complete";
	}

	if(total != size) {
		LOG(INFO) << "Publish failed.  Intended bytes: " << size << " actual write: " << total;
 		throw PacketPublishFailure(strerror(errno));
	}

	return true;
}




