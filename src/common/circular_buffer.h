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

#ifndef __CIRCULAR_BUFFER_H_
#define __CIRCULAR_BUFFER_H_

#include <stddef.h>

class CircularBuffer {
    /********************
     *      METHODS     *
     ********************/
public:
    ///////////////////////
    // Public Methods
	CircularBuffer(size_t capacity);
	~CircularBuffer();

	// Return current size of buffer
	size_t size() const {
		return size_;
	}

    // Return current size of peek buffer
    size_t peek_size() const {
        return peek_size_;
    }

	// Return capacity of buffer
	size_t capacity() const {
		return capacity_;
	}

	// Return available space in buffer
	size_t available() const {
		return capacity_ - size_;
	}
	// Write data to buffer
	size_t write(const char *data, size_t bytes);

	// Read data from buffer
	size_t read(char *data, size_t bytes);

	// Remove data from buffer without copying
	size_t discard(size_t bytes);

	// Clear all data from buffer
	size_t clear();

	// Read data from buffer without removing
	size_t peek(char *data, size_t bytes);

	// Read next byte from buffer without extracting
	size_t peek_next_byte(char &byte);

	// Reset peek index
	void reset_peek();

private:
    ///////////////////////
    // Private Methods

    CircularBuffer();

    /********************
     *      MEMBERS     *
     ********************/

	size_t beg_index_, end_index_, size_, peek_size_, capacity_, peek_index_;
	char *data_;
};

#endif // __CIRCULAR_BUFFER_H_
