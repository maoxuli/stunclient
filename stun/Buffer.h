//
//  Buffer.h
//  network
//
//  Created by Maoxu Li on 8/10/10.
//  Copyright (c) 2010 LIM Labs. All rights reserved.
//

#ifndef NETWORK_BUFFER_H
#define NETWORK_BUFFER_H

#include "Network.h"
#include <string>
#include <vector>
#include <cassert>

NETWORK_BEGIN

//
// Buffer of bytes
// Based on std::vector of unsigned char
//

class Buffer
{
public:
    Buffer(size_t limit = 0); // limit the max size of buffer ?Include bytes before _read_index?
    Buffer(const unsigned char* b, size_t n); // Wrapper of a memory block
    ~Buffer();
    
    // empty the buffer
    void clear()
    {
        _read_index = 0;
        _write_index = 0; // ?Will thrink capacity?
    }
    
    // Swap two buffers without copy
    void swap(Buffer& b)
    {
        _v.swap(b._v);
        std::swap(_read_index, b._read_index);
        std::swap(_write_index, b._write_index);
        std::swap(_max_size, b._max_size);
    }
    
    // Swap two buffers without copy
    void swap(Buffer* b)
    {
        _v.swap(b->_v);
        std::swap(_read_index, b->_read_index);
        std::swap(_write_index, b->_write_index);
        std::swap(_max_size, b->_max_size);
    }
    
    // bytes number of the buffer
    // size() = readable() + writable()
    size_t size() const
    {
        return _v.size() - _read_index;
    }
    
    // available bytes to read
    size_t readable() const
    {
        return _write_index - _read_index;
    }
    
    // free bytes to write
    size_t writable() const
    {
        return _v.size() - _write_index;
    }
    
    //
    // Write data to buffer
    // Always append data immediately after readable data
    //
    
    // unsigned char src[100];
    // buf.reserve(100);
    // memcpy(buf.write(), src, 100);
    // buf.write(100);
    //
    // or:
    // unsigned char src[100];
    // buf.writeBlob(src, 100);
    
    unsigned char* write()
    {
        return begin() + _write_index;
    }
    
    const unsigned char* write() const
    {
        return begin() + _write_index;
    }
    
    void write(size_t n)
    {
        assert(_write_index + n <= _v.size());
        _write_index += n;
    }
    
    size_t reserve(size_t n)
    {
        if(n == 0 || writable() >= n)
        {
            return writable();
        }
        
        if(_max_size > 0 && (_v.size() + n) > _max_size)
        {
            _v.resize(_max_size);
        }
        else
        {
            _v.resize(_v.size() + n);
        }
        
        return writable();
    }
    
    // Write particular data type
    size_t write8(int8_t v);
    size_t write8u(uint8_t v);
    
    size_t write16(int16_t v);
    size_t write16u(uint16_t v);
    
    size_t write32(int32_t v);
    size_t write32u(uint32_t v);
    
    size_t write64(int64_t v);
    size_t write64u(uint64_t v);
    
    size_t writeBool(bool v);
    size_t writeFloat(float v);
    size_t writeDouble(double v);
    
    size_t writeString(const std::string& s);
    size_t writeString(const std::string& s, const char delim);
    size_t writeString(const std::string& s, const std::string& delim);
    
    size_t writeBlob(const unsigned char* b, size_t n);

    //
    // Read data in buffer
    // Always read from the first readable byte
    // The data will be removed from buffer after read
    //
    
    // unsigned char dst[20];
    // memcpy(dst, buf.read(), 20);
    // buf.read(20);
    // [or buf.remove(20);]
    //
    // or:
    // unsigned char dst[20];
    // buf.readBlob(dst, 20);
    
    const unsigned char* read() const
    {
        return begin() + _read_index;
    }
    
    void read(size_t n)
    {
        if(readable() > n)
        {
            _read_index += n;
        }
        else
        {
            _read_index = 0;
            _write_index = 0;
        }
    }
    
    void remove(size_t n)
    {
        read(n);
    }
    
    // Read with particular data type
    int8_t read8();
    uint8_t read8u();
    
    int16_t read16();
    uint16_t read16u();
    
    int32_t read32();
    uint32_t read32u();
    
    int64_t read64();
    uint64_t read64u();
    
    bool readBool();
    float readFloat();
    double readDouble();
    
    bool readString(std::string& s, size_t n);
    bool readString(std::string& s, const char delim);
    bool readString(std::string& s, const std::string& delim);
    
    bool readBlob(unsigned char* b, size_t n);
    
    //
    // Peek data in buffer
    // Data is not removed from the buffer
    // Can peek data at a random position
    //
    
    const unsigned char* peek(size_t offset = 0) const
    {
        return readable() > offset ? read() + offset : NULL;
    }
    
    // Peek particular data type
    int8_t peek8(size_t offset = 0) const;
    uint8_t peek8u(size_t offset = 0) const;
    
    int16_t peek16(size_t offset = 0) const;
    uint16_t peek16u(size_t offset = 0) const;
    
    int32_t peek32(size_t offset = 0) const;
    uint32_t peek32u(size_t offset = 0) const;
    
    int64_t peek64(size_t offset = 0) const;
    uint64_t peek64u(size_t offset = 0) const;
    
    bool peekBool(size_t offset = 0) const;
    float peekFloat(size_t offset = 0) const;
    double peekDouble(size_t offset = 0) const;
    
    bool peekString(std::string& s, size_t n, size_t offset = 0) const;
    bool peekString(std::string& s, const char delim, size_t offset = 0) const;
    bool peekString(std::string& s, const std::string& delim, size_t offset = 0) const;
    
    //
    // Update data in buffer
    // Particular data type and offset determine the bytes that will be modified
    // data is updated based on the same structure (size)
    //
    
    // Update particular data type
    void update8(int8_t v, size_t offset = 0);
    void update8u(uint8_t v, size_t offset = 0);
    
    void update16(int16_t v, size_t offset = 0);
    void update16u(uint16_t v, size_t offset = 0);
    
    void update32(int32_t v, size_t offset = 0);
    void update32u(uint32_t v, size_t offset = 0);
    
    void update64(int64_t v, size_t offset = 0);
    void update64u(uint64_t v, size_t offset = 0);
    
    void updateBool(bool v, size_t offset = 0);
    void updateFloat(float v, size_t offset = 0);
    void updateDouble(double v, size_t offset = 0);
    
    // Receive data from socket and write into buffer
    ssize_t receive(SOCKET fd);
    ssize_t receiveFrom(SOCKET fd, struct sockaddr_storage* from);
    
    // Send data in buffer to socket
    ssize_t send(SOCKET fd);
    ssize_t sendTo(SOCKET fd, struct sockaddr_storage* to);
    
private:
    
    //
    //        |                        -size()-                          |
    // vector |##########################################################|-------------|
    //      begin()                                                     end()       capacity
    //
    //
    //
    //                 |               -size()-                          |
    // Buffer |--------|#########################|***********************|-------------|
    //                        -readable()-              -writable()-
    //      begin() read_index               write_index                end()       capacity
    //
    //
    
    unsigned char* begin()
    {
        return &_v[0];
    }
    
    const unsigned char* begin() const
    {
        return &_v[0];
    }

    std::vector<unsigned char> _v;
    size_t _max_size; // Limitation of the size of the container
    size_t _read_index; // Index of first readable
    size_t _write_index; // Index of first writable
};

NETWORK_END

#endif 

