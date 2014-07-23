/*******************************************************************************
 * Class: CircularBuffer
 * Filename: circular_buffer.h
 * Author: Kevin Stiemke (stiemke27@gmail.com)
 * License: Apache 2.0
 *
 * A circular buffer to store raw bytes of arbitrary length.
 *
 * Usage:
 *
 * CircularBuffer buffer(capacity);
 *
 * size_t bytesWritten = buffer.write(char* data, numBytes)
 * size_t bytesRead = buffer.read(char* data, numBytes)
 *
 ******************************************************************************/

#include "circular_buffer.h"

#include <algorithm>

/******************************************************************************
 *   PUBLIC METHODS
 ******************************************************************************/
/******************************************************************************
 * Method: Constructor
 * Description: Constructor
 * Parameters:
 *   capacity - buffer capacity
 ******************************************************************************/
CircularBuffer::CircularBuffer(size_t capacity) :
		beg_index_(0), end_index_(0), size_(0), peek_size_(0), peek_index_(0), capacity_(
				capacity) {

	data_ = new char[capacity];
}

/******************************************************************************
 * Method: Destructor
 * Description: free up buffer
 ******************************************************************************/
CircularBuffer::~CircularBuffer() {
	delete[] data_;
}

/******************************************************************************
 * Method: write
 * Description: write data to buffer
 * Parameters:
 *   data - pointer to data to store in buffer
 *   bytes - number of bytes to store in buffer
 * Return:
 *   number of bytes stored in buffer.  will be less than number of bytes param
 *   if buffer is full.
 ******************************************************************************/
size_t CircularBuffer::write(const char *data, size_t bytes) {
	if ((bytes == 0) || (available() == 0))
		return 0;

	size_t bytes_to_write = std::min(bytes, capacity_ - size_);

	// Write in a single step
	if (bytes_to_write <= capacity_ - end_index_) {
		memcpy(data_ + end_index_, data, bytes_to_write);
		end_index_ += bytes_to_write;
		if (end_index_ == capacity_)
			end_index_ = 0;
	}
	// Write in two steps
	else {
		size_t size_1 = capacity_ - end_index_;
		memcpy(data_ + end_index_, data, size_1);
		size_t size_2 = bytes_to_write - size_1;
		memcpy(data_, data + size_1, size_2);
		end_index_ = size_2;
	}

	size_ += bytes_to_write;
	peek_size_ += bytes_to_write;
	return bytes_to_write;
}

/******************************************************************************
 * Method: read
 * Description: read data from buffer
 * Parameters:
 *   data - pointer to memory to copy bytes into
 *   bytes - number of bytes to read from buffer
 * Return:
 *   number of bytes read from buffer.  will be less than number of bytes param
 *   if buffer is empty.
 ******************************************************************************/
size_t CircularBuffer::read(char *data, size_t bytes) {
    if ((bytes == 0) || (size_ == 0))
		return 0;

	size_t capacity = capacity_;
	size_t bytes_to_read = std::min(bytes, size_);

	// Read in a single step
	if (bytes_to_read <= capacity - beg_index_) {
		memcpy(data, data_ + beg_index_, bytes_to_read);
		beg_index_ += bytes_to_read;
		if (beg_index_ == capacity)
			beg_index_ = 0;
	}
	// Read in two steps
	else {
		size_t size_1 = capacity - beg_index_;
		memcpy(data, data_ + beg_index_, size_1);
		size_t size_2 = bytes_to_read - size_1;
		memcpy(data + size_1, data_, size_2);
		beg_index_ = size_2;
	}

    peek_index_ = beg_index_;
	size_ -= bytes_to_read;
	peek_size_ = size_;
	return bytes_to_read;
}

/******************************************************************************
 * Method: discard
 * Description: remove data from buffer without copying
 * Parameters:
 *   bytes - number of bytes to remove
 * Return:
 *   number of bytes removed from buffer.  will be less than number of bytes
 *   param if buffer is empty.
 ******************************************************************************/
size_t CircularBuffer::discard(size_t bytes) {
	if ((bytes == 0) || (size_ == 0))
		return 0;

	size_t capacity = capacity_;
	size_t bytes_to_discard = std::min(bytes, size_);

	// Discard in a single step
	if (bytes_to_discard <= capacity - beg_index_) {
		beg_index_ += bytes_to_discard;
		if (beg_index_ == capacity)
			beg_index_ = 0;
	}
	// Discard in two steps
	else {
		size_t size_1 = capacity - beg_index_;
		size_t size_2 = bytes_to_discard - size_1;
		beg_index_ = size_2;
	}

	peek_index_ = beg_index_;
	size_ -= bytes_to_discard;
	peek_size_ = size_;
	return bytes_to_discard;
}

/******************************************************************************
 * Method: peek
 * Description: read data from buffer without removing.  next peek will
 *              retrieve subsequent data from buffer until peek_reset()
 * Parameters:
 *   data - pointer to memory to copy bytes into
 *   bytes - number of bytes to read from buffer
 * Return:
 *   number of bytes read from buffer.  will be less than number of bytes param
 *   if buffer is empty.
 ******************************************************************************/
size_t CircularBuffer::peek(char *data, size_t bytes) {
    if ((bytes == 0) || (peek_size_ == 0))
		return 0;

	size_t bytes_to_read = std::min(bytes, peek_size_);

	// Read in a single step
	if (bytes_to_read <= capacity_ - peek_index_) {
		memcpy(data, data_ + peek_index_, bytes_to_read);
		peek_index_ += bytes_to_read;
		if (peek_index_ == capacity_)
			peek_index_ = 0;
	}
	// Read in two steps
	else {
		size_t size_1 = capacity_ - peek_index_;
		memcpy(data, data_ + peek_index_, size_1);
		size_t size_2 = bytes_to_read - size_1;
		memcpy(data + size_1, data_, size_2);
		peek_index_ = size_2;
	}

	peek_size_ -= bytes_to_read;
	return bytes_to_read;
}

/******************************************************************************
 * Method: peek_next_byte
 * Description: read byte from buffer without removing.  next peek will
 *              retrieve subsequent data from buffer until peek_reset().
 *              read() and discard() will also reset peek.
 * Parameters:
 *   byte - byte to copy into
 * Return:
 *   1 if buffer is not empty else 0
 ******************************************************************************/
size_t CircularBuffer::peek_next_byte(char &byte) {
    if (peek_size_ == 0)  return 0;
    if (peek_index_ == capacity_) peek_index_ = 0;
    byte = *(data_ + peek_index_);
    peek_index_++;
    peek_size_--;
    return 1;
}

/******************************************************************************
 * Method: reset_peek
 * Description: reset peek index into buffer
 ******************************************************************************/
void CircularBuffer::reset_peek() {
	peek_index_ = beg_index_;
	peek_size_ = size_;
}

/******************************************************************************
 * Method: clear
 * Description: remove all data from buffer
 ******************************************************************************/
size_t CircularBuffer::clear() {
    return discard(size());
}

