//
//  Message.cpp
//  stun
//
//  Created by Maoxu Li on 2/11/12.
//  Copyright (c) 2012 LIM Labs. All rights reserved.
//

#include "Message.h"
#include <iostream>

STUN_BEGIN

Attribute::Attribute(unsigned short type, size_t length)
: _type(type)
, _length(length)
{
    
}

Attribute::~Attribute()
{
    
}

unsigned short Attribute::type() const
{
    return _type;
}

size_t Attribute::length() const
{
    return _length;
}

unsigned short Attribute::checkType(network::Buffer* buf)
{
    return ntohs(buf->peek16u());
}

size_t Attribute::toBuffer(network::Buffer* buf) const
{
    // Type + Length
    size_t len = buf->write16u(htons(_type));
    len += buf->write16u(htons(length()));
    
    // Value
    len += valueToBuffer(buf);
    return len;
}

bool Attribute::fromBuffer(network::Buffer* buf)
{
    // Type + Value
    unsigned short type = ntohs(buf->read16u());
    assert(type == _type);
    
    _length = ntohs(buf->read16u());
    assert(buf->readable() >= _length);

    return valueFromBuffer(buf);
}

size_t Attribute::valueToBuffer(network::Buffer* buf) const
{
    // Write ambigious data with '_length' size
    buf->reserve(_length);
    buf->write(_length);
    return _length;
}

bool Attribute::valueFromBuffer(network::Buffer* buf)
{
    // Read data of '_length' size
    if(buf->readable() >= _length)
    {
        buf->read(_length);
        return true;
    }
    return false;
}

///////////////////////////////////////////////////////////////////////

Message::Message(MESSAGE_TYPE type)
: _type(type)
{

}

Message::Message(MESSAGE_TYPE type, const network::UUID& tid)
: _type(type)
, _tid(tid)
{

}

Message::~Message()
{
    for(int i = 0; i < _attributes.size(); ++i)
    {
        delete _attributes[i];
    }
    _attributes.clear();
}

MESSAGE_TYPE Message::type() const
{
    return _type;
}

size_t Message::length() const
{
    size_t len = 0; //MESSAGE_HEADER_LENGTH;
    for(int i = 0; i < _attributes.size(); ++i)
    {
        len += ATTRIBUTE_HEADER_LENGTH;
        len += _attributes[i]->length();
    }
    return len;
}

network::UUID Message::tid() const 
{
    return _tid;
}

std::string Message::toString() const
{
    return _tid.toString();
}

void Message::setAttribute(Attribute* attribute)
{
    _attributes.push_back(attribute);
}

Attribute* Message::findAttribute(ATTRIBUTE_TYPE type) const
{
    for(int i = 0; i < _attributes.size(); ++i)
    {
        if(_attributes[i]->type() == type)
        {
            return _attributes[i];
        }
    }
    return NULL;
}

unsigned short Message::checkType(network::Buffer* buf)
{
    return ntohs(buf->peek16u());
}

size_t Message::toBuffer(network::Buffer* buf) const
{
    size_t len = 0;
    
    // Header
    len += buf->write16u(htons(_type));
    len += buf->write16u(htons(length()));
    len += buf->writeBlob(_tid.bytes(), _tid.size());
    
    // Attributes
    for(int i = 0; i < _attributes.size(); ++i)
    {
        len += _attributes[i]->toBuffer(buf);
    }
    
    return len;
}

bool Message::fromBuffer(network::Buffer* buf)
{
    assert(buf->readable() >= MESSAGE_HEADER_LENGTH);
    if(buf->readable() < MESSAGE_HEADER_LENGTH)
    {
        return false;
    }
    
    unsigned short type = ntohs(buf->read16u());
    assert(type == _type);
    
    size_t length = ntohs(buf->read16u()); // Memory length of attributes
    _tid = network::UUID(buf->read(), buf->readable());
    assert(_tid.size() == 16);
    buf->read(16);
    
    assert(buf->readable() >= length);
    if(buf->readable() < length)
    {
        return false;
    }
    
    while(length > 0)
    {
        Attribute* attribute = AttributeFactory::fromBuffer(buf);
        if(attribute != NULL)
        {
            _attributes.push_back(attribute);
            length -= ATTRIBUTE_HEADER_LENGTH + attribute->length();
        }
        else
        {
            break;
        }
    }
    assert(length == 0);
    return true;
}

/////////////////////////////////////////////////////////////////////////////

Message* MessageFactory::fromBuffer(network::Buffer* buf)
{
    assert(buf != NULL);
    unsigned short type = Message::checkType(buf);
    switch (type)
    {
        case MT_BINDING_RESPONSE:
        {
            BindingResponse* msg = new BindingResponse();
            msg->fromBuffer(buf);
            return msg;
        }
        break;
        case MT_BINDING_ERROR_RESPONSE:
        {
            BindingErrorResponse* msg = new BindingErrorResponse();
            msg->fromBuffer(buf);
            return msg;
        }
        break;
            
        default:
            assert(false);
            break;
    }
    return NULL;
}

/////////////////////////////////////////////////////////////////////////////

BindingRequest::BindingRequest()
: Message(MT_BINDING_REQUEST, network::UUID::New())
{
    
}

BindingRequest::~BindingRequest()
{
    
}

void BindingRequest::setResponseAddress(const sockaddr_in& sa)
{
    AddressAttribute* aa = new AddressAttribute(AT_RESPONSE_ADDRESS, sa);
    setAttribute(aa);
}

void BindingRequest::setChangeRequest(bool port, bool ip)
{
    ChangeRequestAttribute* cra = new ChangeRequestAttribute(port, ip);
    setAttribute(cra);
}

void BindingRequest::setUserName(const std::string &name)
{

}

void BindingRequest::setMessageIntegrity()
{
    // Calculate HMAC and set MessageIntegrityAttribute
}

//////////////////////////////////////////////////////////////////////////////////////////

BindingResponse::BindingResponse()
: Message(MT_BINDING_RESPONSE)
{
    
}
BindingResponse::~BindingResponse()
{
    
}

// Attributes
sockaddr_in BindingResponse::mappedAddress() const
{
    AddressAttribute* aa = dynamic_cast<AddressAttribute*>(findAttribute(AT_MAPPED_ADDRESS));
    if(aa != NULL)
    {
        return aa->address();
    }
    return sockaddr_in();
}

sockaddr_in BindingResponse::sourceAddress() const
{
    AddressAttribute* aa = dynamic_cast<AddressAttribute*>(findAttribute(AT_SOURCE_ADDRESS));
    if(aa != NULL)
    {
        return aa->address();
    }
    return sockaddr_in();
}

sockaddr_in BindingResponse::changedAddress() const
{
    AddressAttribute* aa = dynamic_cast<AddressAttribute*>(findAttribute(AT_CHANGED_ADDRESS));
    if(aa != NULL)
    {
        return aa->address();
    }
    return sockaddr_in();
}

sockaddr_in BindingResponse::reflectedFrom() const
{
    AddressAttribute* aa = dynamic_cast<AddressAttribute*>(findAttribute(AT_REFLECTED_FROM));
    if(aa != NULL)
    {
        return aa->address();
    }
    return sockaddr_in();
}

bool BindingResponse::messageIntegrity() const
{
    return true;
}

//////////////////////////////////////////////////////////////////////////////////////////

/* RFC 3489 
 0                   1                   2                   3
 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 |                   0                     |Class|     Number    |
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 |      Reason Phrase (variable)                                ..
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 */

BindingErrorResponse::BindingErrorResponse()
: Message(MT_BINDING_ERROR_RESPONSE)
{
    
}

BindingErrorResponse::~BindingErrorResponse()
{
    
}

////////////////////////////////////////////////////////////////////////////////////////////

/*
 To allow future revisions of this specification to add new attributes
 if needed, the attribute space is divided into optional and mandatory
 ones.  Attributes with values greater than 0x7fff are optional, which
 means that the message can be processed by the client or server even
 though the attribute is not understood.  Attributes with values less
 than or equal to 0x7fff are mandatory to understand, which means that
 the client or server cannot process the message unless it understands
 the attribute.
 */

Attribute* AttributeFactory::fromBuffer(network::Buffer* buf)
{
    Attribute* a = NULL;
    unsigned short type = Attribute::checkType(buf);
    switch (type)
    {
        case AT_MAPPED_ADDRESS:
        case AT_RESPONSE_ADDRESS:
        case AT_SOURCE_ADDRESS:
        case AT_CHANGED_ADDRESS:
        case AT_REFLECTED_FROM:
            a = new AddressAttribute(static_cast<ATTRIBUTE_TYPE>(type));
            break;
            
        default:
            a = new Attribute(static_cast<ATTRIBUTE_TYPE>(type));
            break;
    }
    assert(a != NULL);
    a->fromBuffer(buf);
    return a;
}

////////////////////////////////////////////////////////////////////////////////////////////

/*
 It consists of an eight bit address family, and a sixteen bit
 port, followed by a fixed length value representing the IP address.
 
 0                   1                   2                   3
 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 |x x x x x x x x|    Family     |           Port                |
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 |                             Address                           |
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 */

AddressAttribute::AddressAttribute(ATTRIBUTE_TYPE type)
: Attribute(type, 8) // Memory length is fixed to 8 bytes
{
    memset(&_address, 0, sizeof(_address));
}

AddressAttribute::AddressAttribute(ATTRIBUTE_TYPE type, const sockaddr_in& sa)
: Attribute(type, 8) // Memory length is fixed to 8 bytes
, _address(sa)
{
    memset(&_address, 0, sizeof(_address));
}

AddressAttribute::~AddressAttribute()
{
    
}

sockaddr_in AddressAttribute::address() const
{
    return _address;
}

// Pack into buffer
size_t AddressAttribute::valueToBuffer(network::Buffer* buf) const
{
    size_t len = buf->write8u(0); // padding
    len += buf->write8u(_address.sin_family);
    len += buf->write16u(_address.sin_port);
    len += buf->write32u(_address.sin_addr.s_addr);
    assert(len == _length);
    return len;
}

// Parse from buffer
bool AddressAttribute::valueFromBuffer(network::Buffer* buf)
{
    if(buf->readable() >= length())
    {
        buf->read8u(); // Discard first 8 bits padding
        _address.sin_family = buf->read8u();
        _address.sin_port = buf->read16u();
        _address.sin_addr.s_addr = buf->read32u();
        return true;
    }
    std::cout << "Buffer size after address read: " << buf->readable() << "\n";
    return false;
}

///////////////////////////////////////////////////////////////////////////

/*
 The CHANGE-REQUEST attribute is used by the client to request that
 the server use a different address and/or port when sending the
 response.  The attribute is 32 bits long, although only two bits (A
 and B) are used:
 
 0                   1                   2                   3
 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 |0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 A B 0|
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 
 The meaning of the flags is:
 
 A: This is the "change IP" flag.  If true, it requests the server
 to send the Binding Response with a different IP address than the
 one the Binding Request was received on.
 
 B: This is the "change port" flag.  If true, it requests the
 server to send the Binding Response with a different port than the
 one the Binding Request was received on.
 */

ChangeRequestAttribute::ChangeRequestAttribute(bool port, bool ip)
: Attribute(AT_CHANGE_REQUEST, 4) // Memory length is fixed to 4 bytes
, _portChange(port)
, _ipChange(ip)
{

}

ChangeRequestAttribute::~ChangeRequestAttribute()
{
    
}

size_t ChangeRequestAttribute::valueToBuffer(network::Buffer* buf) const
{
    unsigned int value = 0;
    value |= _portChange ? 2 : 0;
    value |= _ipChange ? 4 : 0;
    size_t len = buf->write32u(htonl(value));
    assert(len == _length);
    return len;
}

bool ChangeRequestAttribute::valueFromBuffer(network::Buffer* buf)
{
    if(buf->readable() >= length())
    {
        unsigned int value = ntohl(buf->read32u());
        _portChange = (value & 2) >> 1;
        _ipChange = (value & 4) >> 2;
        return true;
    }
    return false;
}

STUN_END
