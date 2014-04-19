//
//  Buffer.cpp
//  network
//
//  Created by Maoxu Li on 8/10/10.
//  Copyright (c) 2010 LIM Labs. All rights reserved.
//

#include "Buffer.h"

NETWORK_BEGIN

// limit is max size
// 0 is no limit
// Note: this limit include the bytes before read index
Buffer::Buffer(size_t limit)
: _v(1024)
, _max_size(limit)
, _read_index(0)
, _write_index(0)
{
    
}

Buffer::Buffer(const unsigned char* b, size_t n)
: _v(2 * n)
, _max_size(0)
, _read_index(0)
, _write_index(0)
{
    memcpy(&_v[0], b, n);
    _write_index = n;
}

Buffer::~Buffer()
{
    
}

// Output data to socket
ssize_t Buffer::send(SOCKET fd)
{
    assert(fd != INVALID_SOCKET);
    
    ssize_t count = 0;
    while(readable() > 0)
    {
        ssize_t n = ::send(fd, read(), readable(), 0);
        if(n <= 0)
        {
            break;
        }
    
        read(n);
        count += n;
    }
    
    return count;
}

// Datagram socket
// Todo: limit the size of datagram
ssize_t Buffer::sendTo(SOCKET fd, struct sockaddr_storage* to)
{
    assert(fd != INVALID_SOCKET);
    assert(to != NULL);
    
    socklen_t tolen = static_cast<socklen_t>(sizeof(sockaddr_storage));
    if(to->ss_family == PF_INET)
    {
        tolen = sizeof(sockaddr_in);
    }
    else if(to->ss_family == PF_INET6)
    {
        tolen = sizeof(sockaddr_in6);
    }
    else
    {
        assert(false);
    }
    
    ssize_t n = ::sendto(fd, read(), readable(), 0, reinterpret_cast<struct sockaddr*>(to), tolen);
    if(n > 0)
    {
        read(n);
    }
    
    return n;
}

// Input data from socket
ssize_t Buffer::receive(SOCKET fd)
{
    assert(fd != INVALID_SOCKET);
    
    ssize_t count = 0;
    while(true)
    {
        reserve(512);
        ssize_t n = ::recv(fd, (char*)write(), writable(), 0);
        if(n <= 0)
        {
            break;
        }
        
        write(n);
        count += n;
    }
    
    return count;
}

// Receive from datagram socket
// Todo: Size limit of datagram
ssize_t Buffer::receiveFrom(SOCKET fd, struct sockaddr_storage* from)
{
    assert(fd != INVALID_SOCKET);
    assert(from != NULL);
    
    reserve(512);
    memset(from, 0, sizeof(struct sockaddr_storage));
    socklen_t fromlen = static_cast<socklen_t>(sizeof(sockaddr_storage));
    ssize_t n = ::recvfrom(fd, write(), writable(), 0, reinterpret_cast<struct sockaddr*>(from), &fromlen);
    if(n > 0)
    {
        write(n);
    }
    
    return n;
}

// Write particular data type
size_t Buffer::write8(int8_t v)
{
    size_t n = sizeof(v);
    assert(n == 1);
    assert(writable() >= n);
    
    int8_t* dest = reinterpret_cast<int8_t*>(write());
    *dest = v;
    write(n);
    return n;
}

size_t Buffer::write8u(uint8_t v)
{
    size_t n = sizeof(v);
    assert(n == 1);
    assert(writable() >= n);
    
    uint8_t* dest = reinterpret_cast<uint8_t*>(write());
    *dest = v;
    write(n);
    return n;
}

size_t Buffer::write16(int16_t v)
{
    size_t n = sizeof(v);
    assert(n == 2);
    assert(writable() >= n);
    
    int16_t* dest = reinterpret_cast<int16_t*>(write());
    *dest = v;
    write(n);
    return n;
}

size_t Buffer::write16u(uint16_t v)
{
    size_t n = sizeof(v);
    assert(n == 2);
    assert(writable() >= n);
    
    uint16_t* dest = reinterpret_cast<uint16_t*>(write());
    *dest = v;    write(n);
    return n;
}

size_t Buffer::write32(int32_t v)
{
    size_t n = sizeof(v);
    assert(n == 4);
    assert(writable() >= n);
    
    int32_t* dest = reinterpret_cast<int32_t*>(write());
    *dest = v;
    write(n);
    return n;
}

size_t Buffer::write32u(uint32_t v)
{
    size_t n = sizeof(v);
    assert(n == 4);
    assert(writable() >= n);
    
    uint32_t* dest = reinterpret_cast<uint32_t*>(write());
    *dest = v;
    write(n);
    return n;
}

size_t Buffer::write64(int64_t v)
{
    size_t n = sizeof(v);
    assert(n == 8);
    assert(writable() >= n);
    
    int64_t* dest = reinterpret_cast<int64_t*>(write());
    *dest = v;
    write(n);
    return n;
}

size_t Buffer::write64u(uint64_t v)
{
    size_t n = sizeof(v);
    assert(n == 8);
    assert(writable() >= n);
    
    uint64_t* dest = reinterpret_cast<uint64_t*>(write());
    *dest = v;
    write(n);
    return n;
}

size_t Buffer::writeBool(bool v)
{
    return write8(v ? 1 : 0);
}

size_t Buffer::writeFloat(float v)
{
    size_t n = sizeof(v);
    assert(n == 4);
    assert(writable() >= n);
    
    float* dest = reinterpret_cast<float*>(write());
    *dest = v;
    write(n);
    return n;
}

size_t Buffer::writeDouble(double v)
{
    size_t n = sizeof(v);
    assert(n == 8);
    assert(writable() >= n);
    
    double* dest = reinterpret_cast<double*>(write());
    *dest = v;
    write(n);
    return n;
}

// No delimit is appended
size_t Buffer::writeString(const std::string& v)
{
    size_t n = v.size();
    assert(writable() >= n);
    if(n > 0)
    {
        memcpy(write(), v.data(), n);
        write(n);
    }
    return n;
}

// Append delimit if the string is not terminated with the delimit
size_t Buffer::writeString(const std::string& v, const char delim)
{
    size_t n = writeString(v);
    
    if(v.empty() || (v[v.size() - sizeof(delim)] != delim))
    {
        n += write8(delim);
    }
    
    return n;
}

// Append delimit if the string is not terminated with the delimit
size_t Buffer::writeString(const std::string& v, const std::string& delim)
{
    size_t n = writeString(v);
    
    assert(!delim.empty());
    if(v.empty() || (v.substr(v.size() - delim.size()) != delim))
    {
        n += writeString(delim);
    }
    
    return n;
}

// Write with a byte array
size_t Buffer::writeBlob(const unsigned char* b, size_t n)
{
    assert(writable() >= n);
    memcpy(write(), b, n);
    write(n);
    return n;
}

// Read particular data type
int8_t Buffer::read8()
{
    int8_t v = 0;            // result value
    size_t n = sizeof(v);    // bytes number
    assert(n == 1);
    assert(readable() >= n);
    
    const int8_t* src = reinterpret_cast<const int8_t*>(read());
    v = *src;
    read(n);
    return v;
}

uint8_t Buffer::read8u()
{
    uint8_t v = 0;           // result value
    size_t n = sizeof(v);    // bytes number
    assert(n == 1);
    assert(readable() >= n);
    
    const uint8_t* src = reinterpret_cast<const uint8_t*>(read());
    v = *src;
    read(n);
    return v;
}

int16_t Buffer::read16()
{
    int16_t v = 0;           // result value
    size_t n = sizeof(v);    // bytes number
    assert(n == 2);
    assert(readable() >= n);
    
    const int16_t* src = reinterpret_cast<const int16_t*>(read());
    v = *src;
    read(n);
    return v;
}

uint16_t Buffer::read16u()
{
    uint16_t v = 0;          // result value
    size_t n = sizeof(v);    // bytes number
    assert(n == 2);
    assert(readable() >= n);
    
    const uint16_t* src = reinterpret_cast<const uint16_t*>(read());
    v = *src;
    read(n);
    return v;
}

int32_t Buffer::read32()
{
    int32_t v = 0;           // result value
    size_t n = sizeof(v);    // bytes number
    assert(n == 4);
    assert(readable() >= n);
    
    const int32_t* src = reinterpret_cast<const int32_t*>(read());
    v = *src;
    read(n);
    return v;
}

uint32_t Buffer::read32u()
{
    uint32_t v = 0;           // result value
    size_t n = sizeof(v);    // bytes number
    assert(n == 4);
    assert(readable() >= n);
    
    const uint32_t* src = reinterpret_cast<const uint32_t*>(read());
    v = *src;
    read(n);
    return v;
}

int64_t Buffer::read64()
{
    int64_t v = 0;           // result value
    size_t n = sizeof(v);    // bytes number
    assert(n == 8);
    assert(readable() >= n);
    
    const int64_t* src = reinterpret_cast<const int64_t*>(read());
    v = *src;
    read(n);
    return v;
}

uint64_t Buffer::read64u()
{
    uint64_t v = 0;          // result value
    size_t n = sizeof(v);    // bytes number
    assert(n == 8);
    assert(readable() >= n);
    
    const uint64_t* src = reinterpret_cast<const uint64_t*>(read());
    v = *src;
    read(n);
    return v;
}

float Buffer::readFloat()
{
    float v = 0;           // Value
    size_t n = sizeof(v);   // Bytes number
    assert(n == 4);
    assert(readable() >= n);
    
    const float* src = reinterpret_cast<const float*>(read());
    v = *src;
    read(n);
    return v;
}

double Buffer::readDouble()
{
    double v = 0;           // Value
    size_t n = sizeof(v);   // Bytes number
    assert(n == 8);
    assert(readable() >= n);
    
    const double* src = reinterpret_cast<const double*>(read());
    v = *src;
    read(n);
    return v;
}

bool Buffer::readString(std::string& s, size_t n)
{
    if(readable() < n)
    {
        return false;
    }
    
    const char* p = reinterpret_cast<const char*>(read());
    std::string(p, p + n).swap(s);
    read(n);
    return true;
}

bool Buffer::readString(std::string& s, const char delim)
{
    if(readable() < sizeof(delim))
    {
        return false;
    }
    
    const char* p1 = reinterpret_cast<const char*>(read());
    const char* p2 = std::find(p1, reinterpret_cast<const char*>(write()), delim);
    if(p2 == reinterpret_cast<const char*>(write()))
    {
        return false;
    }
    
    std::string(p1, p2).swap(s);
    read(p2 + sizeof(delim) - p1);
    return true;
}

bool Buffer::readString(std::string& s, const std::string& delim)
{
    if(readable() < delim.size())
    {
        return false;
    }
    
    const char* p1 = reinterpret_cast<const char*>(read());
    const char* p2 = std::find_first_of(p1, reinterpret_cast<const char*>(write()), delim.begin(), delim.end());
    if(p2 == reinterpret_cast<const char*>(write()))
    {
        return false;
    }
    
    std::string(p1, p2).swap(s);
    read(p2 + delim.size() - p1);
    return true;
}

// Read into a bytes array
bool Buffer::readBlob(unsigned char* b, size_t n)
{
    if(readable() < n)
    {
        return false;
    }
    
    memcpy(b, read(), n);
    read(n);
    return true;
}

// Peek particular data type
int8_t Buffer::peek8(size_t offset) const
{
    int8_t v = 0;            // result value
    size_t n = sizeof(v);    // bytes number
    assert(n == 1);
    assert(readable() >= n + offset);
    
    const int8_t* src = reinterpret_cast<const int8_t*>(peek(offset));
    v = *src;
    return v;
}

uint8_t Buffer::peek8u(size_t offset) const
{
    uint8_t v = 0;           // result value
    size_t n = sizeof(v);    // bytes number
    assert(n == 1);
    assert(readable() >= n + offset);
    
    const uint8_t* src = reinterpret_cast<const uint8_t*>(peek(offset));
    v = *src;
    return v;
}

int16_t Buffer::peek16(size_t offset) const
{
    int16_t v = 0;           // result value
    size_t n = sizeof(v);    // bytes number
    assert(n == 2);
    assert(readable() >= n + offset);
    
    const int16_t* src = reinterpret_cast<const int16_t*>(peek(offset));
    v = *src;
    return v;
}

uint16_t Buffer::peek16u(size_t offset) const
{
    uint16_t v = 0;           // result value
    size_t n = sizeof(v);    // bytes number
    assert(n == 2);
    assert(readable() >= n + offset);
    
    const uint16_t* src = reinterpret_cast<const uint16_t*>(peek(offset));
    v = *src;
    return v;
}

int32_t Buffer::peek32(size_t offset) const
{
    int32_t v = 0;          // result value
    size_t n = sizeof(v);    // bytes number
    assert(n == 4);
    assert(readable() >= n + offset);
    
    const int32_t* src = reinterpret_cast<const int32_t*>(peek(offset));
    v = *src;
    return v;
}

uint32_t Buffer::peek32u(size_t offset) const
{
    uint32_t v = 0;          // result value
    size_t n = sizeof(v);    // bytes number
    assert(n == 4);
    assert(readable() >= n + offset);
    
    const uint32_t* src = reinterpret_cast<const uint32_t*>(peek(offset));
    v = *src;
    return v;
}

int64_t Buffer::peek64(size_t offset) const
{
    int64_t v = 0;           // result value
    size_t n = sizeof(v);    // bytes number
    assert(n == 8);
    assert(readable() >= n + offset);
    
    const int64_t* src = reinterpret_cast<const int64_t*>(peek(offset));
    v = *src;
    return v;
}

uint64_t Buffer::peek64u(size_t offset) const
{
    int64_t v = 0;           // result value
    size_t n = sizeof(v);    // bytes number
    assert(n == 8);
    assert(readable() >= n + offset);
    
    const uint64_t* src = reinterpret_cast<const uint64_t*>(peek(offset));
    v = *src;
    return v;
}

bool Buffer::peekBool(size_t offset) const
{
    int8_t v = peek8(offset);
    return v != 0;
}

float Buffer::peekFloat(size_t offset) const
{
    float v = 0;            // Value
    size_t n = sizeof(v);   // Bytes number
    assert(n == 4);
    assert(readable() >= n + offset);
    
    const float* src = reinterpret_cast<const float*>(peek(offset));
    v = *src;
    return v;
}

double Buffer::peekDouble(size_t offset) const
{
    double v = 0;           // Value
    size_t n = sizeof(v);   // Bytes number
    assert(n == 8);
    assert(readable() >= n + offset);
    
    const double* src = reinterpret_cast<const double*>(peek(offset));
    v = *src;
    return v;
}

bool Buffer::peekString(std::string& s, size_t n, size_t offset) const
{
    if(readable() < n + offset)
    {
        return false;
    }

    const char* p = reinterpret_cast<const char*>(peek(offset));
    std::string(p, p + n).swap(s);
    return true;
}

bool Buffer::peekString(std::string& s, const char delim, size_t offset) const
{
    if(readable() < sizeof(delim) + offset)
    {
        return false;
    }
    
    const char* p1 = reinterpret_cast<const char*>(peek(offset));
    const char* p2 = std::find(p1, reinterpret_cast<const char*>(write()), delim);
    if(p2 == reinterpret_cast<const char*>(write()))
    {
        return false;
    }
    
    std::string(p1, p2).swap(s);
    return true;
}

bool Buffer::peekString(std::string& s, const std::string& delim, size_t offset) const
{
    if(readable() < delim.size())
    {
        return false;
    }
    
    const char* p1 = reinterpret_cast<const char*>(peek(offset));
    const char* p2 = std::find_first_of(p1, reinterpret_cast<const char*>(write()), delim.begin(), delim.end());
    if(p2 == reinterpret_cast<const char*>(write()))
    {
        return false;
    }
    
    std::string(p1, p2).swap(s);
    return true;
}

NETWORK_END
